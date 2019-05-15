
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "osqueue.h"


typedef enum { ONLINE, OFFLINE } state;


typedef struct {
    void *args;
    void (*func)(void *);
} Task;

typedef struct {
    int threadNum;
    OSQueue *queue;
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    state state;
} ThreadPool;


// gets num of threads and returns pointer to thread pool
ThreadPool *tpCreate(int threadNum);

// insert task with args to the thread pool
int tpInsertTask(ThreadPool *tp, void (*computeFunc)(void *), void *args);

// destroy the thread pool
void tpDestroy(ThreadPool *tp, int shouldWaitForTasks);


#endif
