#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue Queue;

/* initialize a empty queue */ 
Queue * init_queue();
/* pop the front item of queue, if queue is empty return NULL */
void * queue_pop(Queue *);
/* get the front item of queue, if queue is empty return NULL */
void * queue_front(Queue *);
/* return 1 if queue is empty else return 0 */
int queue_empty(Queue *);
/* push a new item to queue */
void queue_push(Queue *, void *);
/* search an obj in the queue that matches the cmp function */
void * queue_search(Queue *, void *, int (*cmp)(void *, void *));
/* destroy a queue */
void destroy_queue(Queue *q, void (*free_val)(void *));

#endif