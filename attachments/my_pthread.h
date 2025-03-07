#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>

#include "queue.h"

// #define TIME_QUANTUM_MS 50000 // Time quantum in microseconds
#define STACK_SIZE 32768

/* Thread ID */
typedef unsigned int my_pthread_t;

/* Thread Status Enum */
typedef enum threadStatus
{
    RUNNABLE, // Thread can be run
    SLEEP,    // Thread is currently asleep
    FINISHED  // Thread has finished execution
} status_t;

/*  Thread Contol Block */
typedef struct threadControlBlock
{
    my_pthread_t tid;   // Thread ID
    status_t status;    // Thread Status
    ucontext_t context; // Ucontext

    size_t join_times; // the number of threads which it has joined
                        // if it is not 0, it should be remove from ready queue
    char *stack;       // thread stack
    Queue *wait_queue; // threads who joins it
} my_pthread_tcb;

/* Function Declarations */
void my_pthread_create(my_pthread_t *thread, void *(*function)(void *), void *arg);

my_pthread_t my_pthread_self();

void my_pthread_yield();

void my_pthread_exit();

void my_pthread_join(my_pthread_t thread);
