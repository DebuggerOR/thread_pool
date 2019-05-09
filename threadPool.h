
// fogler ori
// 318732484

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "osqueue.h"
#include <string.h>


typedef enum { RUNNING, WAIT_RUNNING, WAIT_ALL } state;


typedef struct {
    void *args;
    void (*func)(void *);
} Task;

typedef struct {
    int threadNum;
    state state;
    OSQueue *queue;
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} ThreadPool;


// gets num of threads and returns pointer to thread pool
ThreadPool *tpCreate(int threadNum);

// insert task with args to the thread pool
int tpInsertTask(ThreadPool *pool, void (*computeFunc)(void *), void *param);

// destroy the thread pool
void tpDestroy(ThreadPool *pool, int shouldWaitForTasks);


#endif



//#ifndef __THREAD_POOL__
//#define __THREAD_POOL__
//
//typedef struct thread_pool
//{
// //The field x is here because a struct without fields
// //doesn't compile. Remove it once you add fields of your own
// int x;
// //TODO - FILL THIS WITH YOUR FIELDS
//}ThreadPool;
//
//ThreadPool* tpCreate(int numOfThreads);
//
//void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);
//
//int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);
//
//#endif