
// fogler ori
// 318732484

#include "threadPool.h"

void sys_error(ThreadPool* threadPool) {
    write(2, "Error in system call\n", strlen("Error in system call\n"));

    // free thread pool if not null
    if(threadPool != NULL){
        free(threadPool);
    }

    exit(1);
}

static void *ThreadFunction(void *param) {
    ThreadPool *pool = (ThreadPool *) param;

    if (pool == NULL){
        sys_error(NULL);
    }

    while (pool->state == RUNNING || (!osIsQueueEmpty(pool->queue) && pool->state == WAIT_STOP)) {
        pthread_mutex_lock(&pool->lock);
        while (pool->state == RUNNING && osIsQueueEmpty(pool->queue)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }
        Task *task = (Task *) osDequeue(pool->queue);
        pthread_mutex_unlock(&pool->lock);
        if (task != NULL) {
            ((task->func))(task->args);
            free(task);
        }
    }
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
}

ThreadPool *tpCreate(int numThreads) {
    // case no positive num threads
    if (numThreads < 1){
        return NULL;
    }

    // else try to create thread pool
    ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (threadPool == NULL) {
        sys_error(NULL);
    }

    // set thread pool fields
    threadPool->state = RUNNING;
    threadPool->numThreads = numThreads;
    threadPool->queue = osCreateQueue();

    //
    if ((pthread_mutex_init(&threadPool->lock, NULL) != 0)
    || (pthread_cond_init(&threadPool->notify, NULL) != 0)) {
        sys_error(NULL);
    }

    // try to create threads
    int threadsSize = sizeof(pthread_t)*(size_t) numThreads;
    threadPool->threads = (pthread_t *) malloc(threadsSize);
    if (threadPool->threads == NULL) {
        sys_error(threadPool);
    }

    //
    int i;
    for (i = 0; i < numThreads; i++) {
        if (pthread_create(&(threadPool->threads[i]), NULL,
                ThreadFunction, (void *) threadPool) != 0) {
            tpDestroy(threadPool, 0);
            sys_error(NULL);
        }
    }

    return threadPool;
}

int tpInsertTask(ThreadPool *pool, void (*computeFunc)(void *), void *param) {
    //
    if (pool->state != RUNNING) {
        return 1;
    }
    //
    Task *task = (Task *) calloc(sizeof(Task), 1);
    if (task == NULL) {
        sys_error(NULL);
    }

    // set task's fields
    task->func = computeFunc;
    task->args = param;
    pthread_mutex_lock(&(pool->lock));

    // insert task to queue
    osEnqueue(pool->queue, task);

    //
    if (pthread_cond_broadcast(&(pool->notify)) != 0) {
        sys_error(NULL);
    }

    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

void tpDestroy(ThreadPool *pool, int shouldWaitForTasks) {
    //
    pthread_mutex_lock(&pool->lock);

    //
    if (shouldWaitForTasks == 0) {
        pool->state = FORCE_STOP;
    } else {
        pool->state = WAIT_STOP;
    }

    //
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&pool->lock);

    // join all threads
    int i;
    for (i = 0; i < pool->numThreads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // free all tasks in queue
    while (!osIsQueueEmpty(pool->queue)) {
        free(osDequeue(pool->queue));
    }

    // free all
    free(pool->threads);
    osDestroyQueue(pool->queue);

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);

    free(pool);
}