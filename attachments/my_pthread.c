#include <assert.h>
#include "my_pthread.h"

/* Scheduler State */

#define BLOCK_ALARM            \
    sigset_t mask, prev_mask;  \
    sigemptyset(&mask);        \
    sigaddset(&mask, SIGALRM); \
    sigprocmask(SIG_BLOCK, &mask, &prev_mask)

#define UNBLOCK_ALARM sigprocmask(SIG_SETMASK, &prev_mask, NULL)

// Fill in Here //
Queue *task_queue, *finished_queue;
static my_pthread_tcb main_tcb;
static int next_tid;
static int first = 1;

static struct itimerval timer_value;

// free exited thread resource
static ucontext_t dummy_context;

static void reset_clock();
static my_pthread_tcb *alloc_tcb(void *(*function)(void *), void *arg);
static void free_tcb(my_pthread_tcb *);
static void *stub(void *(*function)(void *), void *arg);
static int equal(void *, void *);

/* Scheduler Function
 * Pick the next runnable thread and swap contexts to start executing
 */
void schedule(int signum)
{
    // Implement Here
    BLOCK_ALARM;

    my_pthread_tcb *current_tcb, *next_tcb = NULL;
    if (signum != SIGUSR1)
    {
        current_tcb = queue_pop(task_queue);
        next_tcb = NULL;
        queue_push(task_queue, current_tcb);
    }

    // if (current_tcb->status == FINISHED)
    // {
    //     ucontext_t free_context;
    //     getcontext(&free_context);
    //     free_context.uc_stack.ss_sp = stack;
    //     free_context.uc_stack.ss_size = STACK_SIZE;
    //     makecontext(&free_context, free_tcb, 1, current_tcb);
    //     setcontext(&free_context);
    // }
    // else
    // {
    while ((next_tcb = queue_front(task_queue)))
    {
        if (next_tcb->status == RUNNABLE)
            break;
        next_tcb = queue_pop(task_queue);
        assert(next_tcb->status == SLEEP);
        queue_push(task_queue, next_tcb);
    }
    printf("Turn to %d thread %d\n", next_tcb->tid, next_tcb->status);
    ucontext_t *store = (signum == SIGUSR1) ? &dummy_context : &current_tcb->context;
    swapcontext(store, &next_tcb->context);
    // }
    // free finished task
    while ((next_tcb = queue_pop(finished_queue)))
        free_tcb(next_tcb);
    UNBLOCK_ALARM;
}

/* Create a new TCB for a new thread execution context and add it to the queue
 * of runnable threads. If this is the first time a thread is created, also
 * create a TCB for the main thread as well as initalize any scheduler state.
 */
void my_pthread_create(my_pthread_t *thread, void *(*function)(void *), void *arg)
{
    // Implement Here
    my_pthread_tcb *tcb;

    BLOCK_ALARM;

    // The first time you call a library function, do some preparation work
    if (first)
    {
        first = 0;
        // set timer interrupt handler
        if (signal(SIGALRM, schedule) == SIG_ERR)
        {
            perror("Signal error");
            exit(1);
        }

        reset_clock();

        // create a task queue
        task_queue = init_queue();
        finished_queue = init_queue();

        // init main TCB for main thread
        tcb = &main_tcb;
        tcb->status = RUNNABLE;
        tcb->tid = next_tid++;
        queue_push(task_queue, tcb);
        printf("Create Main thread\n");
    }

    // create TCB for new thread and push it
    tcb = alloc_tcb(function, arg);
    queue_push(task_queue, tcb);

    *thread = tcb->tid;
    printf("Create %d thread\n", tcb->tid);

    UNBLOCK_ALARM;
}

/* Give up the CPU and allow the next thread to run.
 */
void my_pthread_yield()
{
    // Implement Here
    BLOCK_ALARM;
    printf("%d thread yield\n", my_pthread_self());
    reset_clock(); // reset clock
    UNBLOCK_ALARM;

    schedule(SIGUSR2);
}

/* The calling thread will not continue until the thread with tid thread
 * has finished executing.
 */
void my_pthread_join(my_pthread_t thread)
{
    // Implement Here //
    BLOCK_ALARM;
    printf("%d want to join %d\n", my_pthread_self(), thread);
    if (!task_queue) // task queue 不存在，则只剩下一个main线程，直接返回即可
    {
        printf("Only Main thread\n");
        UNBLOCK_ALARM;
        return;
    }
    // Moves the thread at the head of the queue to the waiting queue of the specified thread
    my_pthread_tcb *current_tcb = queue_front(task_queue);
    my_pthread_tcb *tcb = queue_search(task_queue, &thread, equal);
    if (tcb)
    {
        printf("%d thread join %d thread\n", my_pthread_self(), thread);
        ++current_tcb->join_times;
        current_tcb->status = SLEEP;
        queue_push(tcb->wait_queue, current_tcb); // add myself to the tcb wait queue
        reset_clock();                            // reset clock
        UNBLOCK_ALARM;

        schedule(SIGUSR2);
    }
    else
    {
        printf("%d thread has exited or no existed\n", thread);
        UNBLOCK_ALARM;
    }
}

/* Returns the thread id of the currently running thread
 */
my_pthread_t my_pthread_self()
{
    // Implement Here //
    my_pthread_t tid;
    BLOCK_ALARM;
    if (task_queue)
    {
        my_pthread_tcb *tcb = queue_front(task_queue);
        tid = tcb->tid;
    }
    else
        tid = 0; // only main thread

    UNBLOCK_ALARM;

    return tid; // temporary return, replace this
}

/* Thread exits, setting the state to finished and allowing the next thread
 * to run.
 */
void my_pthread_exit()
{
    // Implement Here //
    BLOCK_ALARM;
    printf("%d thread exit\n", my_pthread_self());
    my_pthread_tcb *current_tcb = queue_pop(task_queue);
    assert(current_tcb);
    current_tcb->status = FINISHED; // change state to finished
    queue_push(finished_queue, current_tcb);

    // put the waiting threads to task queue
    while (!queue_empty(current_tcb->wait_queue))
    {
        my_pthread_tcb *sleep_tcb = queue_pop(current_tcb->wait_queue);
        --sleep_tcb->join_times;
        if (!sleep_tcb->join_times)
        {
            printf("%d thread awake %d\n", current_tcb->tid, sleep_tcb->tid);
            sleep_tcb->status = RUNNABLE;
            queue_push(task_queue, sleep_tcb);
        }
    }

    UNBLOCK_ALARM;
    schedule(SIGUSR1);
}

static void reset_clock()
{
    timer_value.it_value.tv_sec = 0;
    timer_value.it_value.tv_usec = TIME_QUANTUM_MS;
    timer_value.it_interval.tv_sec = 0;
    timer_value.it_interval.tv_usec = TIME_QUANTUM_MS;
    setitimer(ITIMER_REAL, &timer_value, NULL);
}

static void free_task_queue()
{
    BLOCK_ALARM;
    // 取消timer
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0;
    value.it_interval = value.it_value;
    setitimer(ITIMER_REAL, &value, NULL);
    signal(SIGALRM, SIG_IGN);
    UNBLOCK_ALARM;

    destroy_queue(task_queue, NULL);
    task_queue = NULL;
}

static my_pthread_tcb *alloc_tcb(void *(*function)(void *), void *arg)
{
    my_pthread_tcb *tcb = (my_pthread_tcb *)malloc(sizeof(my_pthread_tcb));
    tcb->wait_queue = init_queue();
    tcb->status = RUNNABLE;
    tcb->tid = next_tid++;
    getcontext(&tcb->context);
    tcb->stack = malloc(STACK_SIZE * sizeof(char));
    tcb->context.uc_stack.ss_sp = tcb->stack;
    tcb->context.uc_stack.ss_size = STACK_SIZE;
    sigemptyset(&tcb->context.uc_sigmask);
    makecontext(&tcb->context, stub, 2, function, arg);

    return tcb;
}

static void free_tcb(my_pthread_tcb *tcb)
{
    printf("%d Free %d\n", ((my_pthread_tcb *)queue_front(task_queue))->tid, tcb->tid);
    free(tcb->stack);

    
    destroy_queue(tcb->wait_queue, NULL);
    free(tcb);

    // ucontext_t *context;
    // my_pthread_tcb *next_tcb = queue_front(task_queue);
    // // 如果当前只有main thread，释放task queue 并设置first 为初始值1
    // if (queue_size(task_queue) == 1 && next_tcb == &main_tcb)
    // {
    //     free_task_queue();
    //     first = 1;
    // }

    // setcontext(&next_tcb->context);
}

static void *stub(void *(*root)(void *), void *arg)
{
    // thread starts here
    root(arg); // call root function
    my_pthread_exit();
}

static int equal(void *a, void *b)
{
    my_pthread_t *tid = (my_pthread_t *)a;
    my_pthread_tcb *tcb = (my_pthread_tcb *)b;

    return *tid == tcb->tid ? 0 : 1;
}