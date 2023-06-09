#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "my_pthread.h"

int sum1, sum2;
int arr[1000];
int flag;

void foo(void *)
{
    for (int i = 0; i < 500; ++i)
    {
        sum1 += arr[i];
        if (i % 2)
            my_pthread_yield();
    }
    flag = 1;
}


void bar(void *)
{
    for (int i = 500; i < 1000; ++i)
    {
        sum2 += arr[i];
        if (i % 2)
            my_pthread_yield();
    }
    flag = 1;
}

int main()
{
    int x;

    my_pthread_t tid1, tid2;

    for (int i = 0; i < 1000; ++i)
        arr[i] = rand() % 10000;
    my_pthread_create(&tid1, (void *)foo, (void *)NULL);
    my_pthread_create(&tid2, (void *)bar, (void *)NULL);
    
    my_pthread_join(tid1);
    my_pthread_join(tid2);

    printf("%d + %d = %d\n", sum1, sum2, sum1 + sum2);

    return 0;
}
