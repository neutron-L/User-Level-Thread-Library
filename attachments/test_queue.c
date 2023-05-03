#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "queue.h"

typedef struct 
{
    /* data */
    long a;
    double d;
    char * str;
} Obj;


static int cmp_int(void *, void *);

static int cmp_obj_a(void *, void *);
static int cmp_obj_d(void *, void *);
static int cmp_obj_str(void *, void *);

static Obj * get_obj(long a, double d, const char * str);
static int equal_obj(Obj *, Obj *);
static void free_obj(void *);

void test_struct_queue()
{
    double d;
    long a;
    char * name;

    Obj * mj = get_obj(23, 30.1, "Jordan");
    Obj * kd = get_obj(35, 27.1, "Durant");
    Obj * sc = get_obj(30, 23.5, "Curry");
    Obj * lbj = get_obj(23, 25.2, "James");
    Obj * jd = get_obj(13, 27.1, "James");

    Queue * q = init_queue();
    Obj * p;

    assert(queue_empty(q));

    queue_push(q, mj); // mj
    d = 30.1;
    p = queue_search(q, &d, cmp_obj_d);
    assert(p && equal_obj(p, mj));
    
    queue_push(q, lbj);   // mj lbj
    a = 23;
    p = queue_search(q, &a, cmp_obj_a);
    assert(p && equal_obj(p, mj));
    
    p = queue_pop(q); // lbj
    assert(p && equal_obj(p, mj));
    queue_push(q, p); // lbj mj

    a = 23;
    p = queue_search(q, &a, cmp_obj_a);
    assert(p && equal_obj(p, lbj));

    a = 30;
    p = queue_search(q, &a, cmp_obj_a);
    assert(!p);

    queue_push(q, kd); // lbj mj kd
    name = "Durant";
    p = queue_search(q, name, cmp_obj_str);
    assert(p && equal_obj(p, kd));

    queue_push(q, sc); // lbj mj kd sc
    a = 30;
    p = queue_search(q, &a, cmp_obj_a);
    assert(p && equal_obj(p, sc));

    d = 30.1;
    p = queue_search(q, &d, cmp_obj_d);
    assert(p && equal_obj(p, mj));

    d = 30.2;
    p = queue_search(q, &d, cmp_obj_d);
    assert(!p);

    queue_push(q, jd);
    name = "James";
    p = queue_search(q, name, cmp_obj_str); // lbj mj kd sc jd
    assert(p && equal_obj(p, lbj));

    p = queue_pop(q);

    name = "James";
    p = queue_search(q, name, cmp_obj_str); // mj kd sc jd
    assert(p && equal_obj(p, jd));

    queue_push(q, lbj); // mj kd sc jd lbj

    a = 23;
    p = queue_search(q, &a, cmp_obj_a); // mj kd sc jd lbj
    assert(p && equal_obj(p, mj));

    p = queue_pop(q);
    assert(p && equal_obj(p, mj));
    queue_push(q, p); // kd sc jd lbj mj

    p = queue_search(q, &a, cmp_obj_a); // mj kd sc jd lbj
    assert(p && equal_obj(p, lbj));

    queue_push(q, mj);

    destroy_queue(q, free_obj);
}


void test_int_queue()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;

    Queue * que = init_queue();
    assert(queue_empty(que)); 
    assert(queue_size(que) == 0);

    queue_push(que, &a);     // 1
    assert(queue_size(que) == 1);
    assert(!queue_empty(que)); 

    int * p = queue_search(que, &a, cmp_int); 
    assert(p && *p == a);
    queue_push(que, &b); // 1 2
    assert(queue_size(que) == 2);
    p = queue_pop(que); // 2
    assert(queue_size(que) == 1);
    queue_push(que, p); // 2 1
    assert(queue_size(que) == 2);
    p = queue_search(que, &b, cmp_int);
    assert(p);
    queue_push(que, p); // 2 1 2
    queue_push(que, &c); // 2 1 2 3
    assert(queue_size(que) == 4);
    p = queue_pop(que);
    assert(*p == 2);
    p = queue_pop(que);
    assert(*p == 1);
    p = queue_pop(que);
    assert(*p == 2);
    p = queue_pop(que);
    assert(*p == 3);

    p = queue_pop(que); //
    assert(!p); 
    assert(queue_size(que) == 0);


    queue_push(que, &d); 

    destroy_queue(que, NULL);
}

int main()
{
    test_int_queue();
    test_struct_queue();
    
    return 0;
}


static int cmp_int(void *a, void *b)
{
    return (*(int *)a == *(int *)b) ? 0 : 1;
}


static int cmp_obj_a(void * a, void * b)
{
    long * p = (long *)a;
    Obj * q = (Obj *)b;

    return (*p == q->a) ? 0 : 1;
}

static int cmp_obj_d(void * a, void * b)
{
    double * p = (double *)a;
    Obj * q = (Obj *)b;

    return (fabs(*p - q->d) < 0.00001) ? 0 : 1;
}

static int cmp_obj_str(void *a, void *b)
{
    char * p = (char *)a;
    Obj * q = (Obj *)b;

    return !strcmp(p, q->str) ? 0 : 1;
}

static Obj * get_obj(long a, double d, const char * str)
{
    Obj * obj = (Obj *)malloc(sizeof(Obj));

    if (obj)
    {
        obj->a = a;
        obj->d = d;
        obj->str = strdup(str);
    }

    return obj;
}

static int equal_obj(Obj * a, Obj * b)
{
    return a->a == b->a && (fabs(a->d - b->d) < 0.00001) && !strcmp(a->str, b->str);
}

static void free_obj(void * a)
{
    free(((Obj *)a)->str);
    free(a);
}