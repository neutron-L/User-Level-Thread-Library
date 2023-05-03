#include "my_pthread.h"

/* Scheduler State */
// Fill in Here //

static int first = 1;

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

  sigprocmask(SIG_SETMASK, &prev_mask, NULL);
}

/* Create a new TCB for a new thread execution context and add it to the queue
 * of runnable threads. If this is the first time a thread is created, also
 * create a TCB for the main thread as well as initalize any scheduler state.
 */
void my_pthread_create(my_pthread_t *thread, void *(*function)(void *), void *arg)
{

  // Implement Here
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
    new_value.it_value.tv_usec = 500000;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_usec = 500000;
    setitimer(ITIMER_REAL, &new_value, &old_value);
    // create TCB for main thread


  }
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
}
