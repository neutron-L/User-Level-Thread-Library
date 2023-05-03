#include <stdlib.h>
#include "queue.h"

typedef struct QNode
{
    void *pval;
    struct QNode *next;
} QNode;

struct Queue
{
    QNode *head;
    QNode *tail;
};

static QNode *free_node_list;

static QNode *get_node();
static void free_node(QNode *);

Queue *init_queue()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (q)
        q->head = q->tail = NULL;
    return q;
}

void *queue_pop(Queue *q)
{
    if (queue_empty(q))
        return NULL;
    void *obj = q->head->pval;
    QNode *tmp = q->head;
    q->head = q->head->next;

    // add free node to free_list
    free_node(tmp);

    if (!q->head)
        q->tail = NULL;
    return obj;
}

void queue_push(Queue *q, void *obj)
{
    QNode *node = get_node();
    node->pval = obj;

    if (q->tail)
        q->tail->next = node;
    q->tail = node;
    if (!q->head)
        q->head = node;
}

int queue_empty(Queue *q)
{
    return !q->head && !q->tail;
}

void *queue_search(Queue *q, void *target, int (*cmp)(void *, void *))
{
    QNode *cur = q->head;
    while (cur && cmp(target, cur->pval))
        cur = cur->next;

    return cur ? cur->pval : NULL;
}

void destroy_queue(Queue *q, void (*free_val)(void *))
{
    QNode *cur = q->head;
    while (cur)
    {
        if (free_val)
            free_val(cur->pval);
        QNode *tmp = cur;
        cur = cur->next;
        free_node(tmp);
    }

    // free free_list
    cur = free_node_list;
    while (cur)
    {
        QNode *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    free_node_list = NULL;

    free(q);
}

static QNode *get_node()
{
    QNode *node = NULL;
    if (free_node_list)
    {
        node = free_node_list;
        free_node_list = free_node_list->next;
        node->next = NULL;
    }
    else
        node = (QNode *)malloc(sizeof(QNode));
    return node;
}

static void free_node(QNode *node)
{
    node->next = free_node_list;
    free_node_list = node;
}