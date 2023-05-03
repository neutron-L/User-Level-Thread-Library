#include <stdio.h>
#include <stdlib.h>
#include "my_pthread.h"

void thread_run()
{
  while (1)
    printf("Other thread\n");
}

int main()
{
  struct itimerval new_value, old_value;
  new_value.it_value.tv_sec = 0;
  new_value.it_value.tv_usec = 500000;
  new_value.it_interval.tv_sec = 0;
  new_value.it_interval.tv_usec = 500000;
  setitimer(ITIMER_REAL, &new_value, &old_value);

  int x;

  my_pthread_t thread;

  my_pthread_create(&thread, (void *)thread_run, (void *)NULL);

  while (1)
    printf("Main thread\n");

  return 0;
}
