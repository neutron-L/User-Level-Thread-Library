#include "my_pthread.h"

/* Scheduler State */
// Fill in Here //
Queue *task_queue;
static int first = 1;
static int next_tid;

static void stub(void *(*function)(void *), void *arg);

/* Scheduler Function
 * Pick the next runnable thread and swap contexts to start executing
 */
void schedule(int signum)
{
    // Implement Here
    sigset_t mask, prev_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);

    my_pthread_tcb *current_tcb = queue_pop(task_queue);
    ucontext_t * current_context;
    if (current_tcb->status == FINISHED)
    {
        free(current_tcb->context.uc_stack.ss_sp);
        free(current_tcb);
        current_context = NULL;
    }
    else
    {
        current_context = &current_tcb->context;
        queue_push(task_queue, current_tcb);
    }
    my_pthread_tcb *next_tcb = queue_front(task_queue);

    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    printf("Alarm\n");
    if (current_context)
        swapcontext(current_context, &next_tcb->context);
    else
        setcontext(&next_tcb->context);
}

/* Create a new TCB for a new thread execution context and add it to the queue
 * of runnable threads. If this is the first time a thread is created, also
 * create a TCB for the main thread as well as initalize any scheduler state.
 */
void my_pthread_create(my_pthread_t *thread, void *(*function)(void *), void *arg)
{

    // Implement Here
    my_pthread_tcb *tcb;

    sigset_t mask, prev_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);

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

        // create TCB for main thread
        tcb = (my_pthread_tcb *)malloc(sizeof(my_pthread_tcb));
        tcb->status = RUNNABLE;
        tcb->tid = next_tid++;
        queue_push(task_queue, tcb);
    }

    // create TCB for new thread and push it
    tcb = (my_pthread_tcb *)malloc(sizeof(my_pthread_tcb));
    tcb->status = RUNNABLE;
    tcb->tid = next_tid++;
    getcontext(&tcb->context);
    tcb->context.uc_stack.ss_sp = malloc(STACK_SIZE * sizeof(char));
    tcb->context.uc_stack.ss_size = STACK_SIZE;
    tcb->context.uc_sigmask = prev_mask;
    makecontext(&tcb->context, stub, 2, function, arg);
    queue_push(task_queue, tcb);

    *thread = tcb->tid;

    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
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
}

/* Returns the thread id of the currently running thread
 */
my_pthread_t my_pthread_self()
{

    // Implement Here //

    return 0; // temporary return, replace this
}

/* Thread exits, setting the state to finished and allowing the next thread
 * to run.
 */
void my_pthread_exit()
{
    // Implement Here //
    sigset_t mask, prev_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);

    my_pthread_tcb *current_tcb = queue_front(task_queue);
    current_tcb->status = FINISHED; // change state to finished

    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    schedule(SIGUSR1);
}

static void stub(void *(*root)(void *), void *arg)
{
    // thread starts here
    root(arg); // call root function
    my_pthread_exit();
}