#include "my_pthread.h"

/* Scheduler State */

#define BLOCK_ALARM sigset_t mask, prev_mask; \
    sigemptyset(&mask); \
    sigaddset(&mask, SIGALRM); \
    sigprocmask(SIG_BLOCK, &mask, &prev_mask)


#define UNBLOCK_ALARM     sigprocmask(SIG_SETMASK, &prev_mask, NULL)


// Fill in Here //
Queue *task_queue;
static my_pthread_tcb main_tcb;
static int next_tid;
static int first = 1;


// free exited thread resource
static char stack[STACK_SIZE];

static my_pthread_tcb * alloc_tcb(void *(*function)(void *), void *arg);
static void free_tcb(my_pthread_tcb *);
static void * stub(void *(*function)(void *), void *arg);
static int equal(void *, void *);

/* Scheduler Function
 * Pick the next runnable thread and swap contexts to start executing
 */
void schedule(int signum)
{
    // Implement Here
    BLOCK_ALARM;

    printf("threads %d\n", queue_size(task_queue));
    my_pthread_tcb *current_tcb = queue_pop(task_queue);
    my_pthread_tcb * next_tcb;

    if (current_tcb->status == FINISHED)
    {
        ucontext_t free_context;
        getcontext(&free_context);
        free_context.uc_stack.ss_sp = stack;
        free_context.uc_stack.ss_size = STACK_SIZE;
        makecontext(&free_context, free_tcb, 1, current_tcb);
        setcontext(&free_context);
    }
    else
    {
        next_tcb = queue_front(task_queue);

        UNBLOCK_ALARM;
        if (current_tcb->status != SLEEP)
        {
            printf("Main\n");
            queue_push(task_queue, current_tcb);
        }
        swapcontext(&current_tcb->context, &next_tcb->context);
    }
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

        struct itimerval new_value, old_value;
        new_value.it_value.tv_sec = 0;
        new_value.it_value.tv_usec = TIME_QUANTUM_MS;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = TIME_QUANTUM_MS;
        setitimer(ITIMER_REAL, &new_value, &old_value);

        // create a task queue
        task_queue = init_queue();


        // init main TCB for main thread
        tcb = &main_tcb;
        tcb->status = RUNNABLE;
        tcb->tid = next_tid++;
        queue_push(task_queue, tcb);
    }

    // create TCB for new thread and push it
    tcb = alloc_tcb(function, arg);
    queue_push(task_queue, tcb);

    *thread = tcb->tid;

    UNBLOCK_ALARM;
}

/* Give up the CPU and allow the next thread to run.
 */
void my_pthread_yield()
{

    // Implement Here
}

/* The calling thread will not continue until the thread with tid thread
 * has finished executing.
 */
void my_pthread_join(my_pthread_t thread)
{
    // Implement Here //
    BLOCK_ALARM;
    // Moves the thread at the head of the queue to the waiting queue of the specified thread
    my_pthread_tcb * current_tcb = queue_front(task_queue);
    current_tcb->status = SLEEP;
    my_pthread_tcb * tcb = queue_search(task_queue, &thread, equal);
    if (tcb)
    {
        queue_push(tcb->wait_queue, current_tcb);
        UNBLOCK_ALARM;
        schedule(SIGUSR2);
    }
    else
        UNBLOCK_ALARM;
}

/* Returns the thread id of the currently running thread
 */
my_pthread_t my_pthread_self()
{
    // Implement Here //
    BLOCK_ALARM;
    my_pthread_tcb *tcb = queue_front(task_queue);
    UNBLOCK_ALARM;

    return tcb->tid; // temporary return, replace this
}

/* Thread exits, setting the state to finished and allowing the next thread
 * to run.
 */
void my_pthread_exit()
{
    // Implement Here //
    BLOCK_ALARM;

    my_pthread_tcb *current_tcb = queue_front(task_queue);
    current_tcb->status = FINISHED; // change state to finished
    printf("Exit\n");
    UNBLOCK_ALARM;
    schedule(SIGUSR1);
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
}


static my_pthread_tcb * alloc_tcb(void *(*function)(void *), void *arg)
{
    my_pthread_tcb * tcb = (my_pthread_tcb *)malloc(sizeof(my_pthread_tcb));
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


static void free_tcb(my_pthread_tcb * tcb)
{
    printf("Free %d\n", tcb->tid);
    free(tcb->stack);

    // put the waiting threads to task queue
    while (!queue_empty(tcb->wait_queue))
    {
        my_pthread_tcb * sleep_tcb = queue_pop(tcb->wait_queue);
        queue_push(task_queue, sleep_tcb);
    }
    destroy_queue(tcb->wait_queue, NULL);
    free(tcb);

    ucontext_t * context;
    my_pthread_tcb * next_tcb = queue_front(task_queue);

    printf("remain %d next %d\n", queue_size(task_queue), next_tcb->tid);
    // 如果当前只有main thread，释放task queue 并设置first 为初始值1
    if (queue_size(task_queue) == 1 && next_tcb == &main_tcb)
    {
        destroy_queue(task_queue, NULL);
        first = 1;
    }

    setcontext(&next_tcb->context);
}

static void * stub(void *(*root)(void *), void *arg)
{
    // thread starts here
    root(arg); // call root function
    my_pthread_exit();
}

static int equal(void * a, void * b)
{
    my_pthread_t * tid = (my_pthread_t *)a;
    my_pthread_tcb * tcb = (my_pthread_tcb *)b;

    return *tid == tcb->tid ? 0 : 1;
}