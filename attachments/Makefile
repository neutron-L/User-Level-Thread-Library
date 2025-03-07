CC = gcc
CFLAGS = -g -DTIME_QUANTUM_MS=50000

all: ThreadRun ThreadJoin MultiThreadRun MultiThreadJoin MultiThreadYield testQueue

ThreadRun: ThreadRun.c my_pthread.h my_pthread.c queue.h queue.c
	$(CC) $(CFLAGS) ThreadRun.c my_pthread.c queue.c -o ThreadRun

ThreadJoin: ThreadJoin.c my_pthread.h my_pthread.c queue.h queue.c 
	$(CC) $(CFLAGS) ThreadJoin.c my_pthread.c queue.c -o ThreadJoin

MultiThreadRun: MultiThreadRun.c my_pthread.h my_pthread.c queue.h queue.c
	$(CC) $(CFLAGS) MultiThreadRun.c my_pthread.c queue.c -o MultiThreadRun

MultiThreadJoin: MultiThreadJoin.c my_pthread.h my_pthread.c queue.h queue.c
	$(CC) $(CFLAGS) MultiThreadJoin.c my_pthread.c queue.c -o MultiThreadJoin

MultiThreadYield: MultiThreadYield.c my_pthread.h my_pthread.c queue.h queue.c
	$(CC) $(CFLAGS) MultiThreadYield.c my_pthread.c queue.c -o MultiThreadYield

testQueue:	test_queue.c queue.c queue.h
	$(CC) $(CFLAGS) queue.c test_queue.c -o testQueue

clean:
	rm -f ThreadRun ThreadJoin MultiThreadRun MultiThreadJoin MultiThreadYield testQueue
