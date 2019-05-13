
// fogler ori
// 318732484

#include "threadPool.h"


// the function displays error message and exits
void sys_error() {
    write(2, "Error in system call\n", strlen("Error in system call\n"));
    exit(-1);
}


// the function gets
static void *exec(void *x) {
    // try to convert to thread pool
    ThreadPool *tp = (ThreadPool *) x;
    if (tp == NULL) {
        sys_error();
    }

    // loop's conditions
    int isRunning = (tp->state == RUNNING);
    int isQueueNotEmpty = !(osIsQueueEmpty(tp->queue));
    int isWaitToAll = (tp->state == WAIT_ALL);

    // while tp running or tp waits to all and queue isn't empty
    while (isRunning || (isWaitToAll && isQueueNotEmpty)) {
        // lock thread pool's mutex
        if(pthread_mutex_lock(&(tp->mutex)) != 0){
            sys_error();
        }

        // block on condition
        while (tp->state == RUNNING && osIsQueueEmpty(tp->queue)) {
            if(pthread_cond_wait(&(tp->condition), &(tp->mutex)) != 0) {
                sys_error();
            }
        }

        // dequeue task from tasks' queue
        Task *task = (Task *) osDequeue(tp->queue);

        // lock thread pool's mutex
        if(pthread_mutex_unlock(&(tp->mutex)) != 0){
            sys_error(tp);
        }

        // do task
        if (task != NULL) {
            ((task->func))(task->args);
        }

        // update loop condition vars
        isRunning = (tp->state == RUNNING);
        isQueueNotEmpty = !(osIsQueueEmpty(tp->queue));
        isWaitToAll = (tp->state == WAIT_ALL);
    }

    pthread_exit(NULL);
}


// the function gets num of threads
// it creates and returns a thread pull with this num of threads
ThreadPool *tpCreate(int threadNum) {
    // case no positive num threads
    if (threadNum < 1){
        return NULL;
    }

    // else try to create thread pool
    ThreadPool *tp = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (tp == NULL) {
        sys_error();
    }

    // set thread pool's fields
    tp->state = RUNNING;
    tp->threadNum = threadNum;
    tp->queue = osCreateQueue();

    // try to init mutex
    if (pthread_mutex_init(&(tp->mutex), NULL) != 0) {
        free(tp);
        sys_error();
    }

    // try to init condition
    if (pthread_cond_init(&(tp->condition), NULL) != 0) {
        free(tp);
        sys_error();
    }

    // try to alloc threads
    int threadsSize = sizeof(pthread_t)*(size_t) threadNum;
    tp->threads = (pthread_t *) malloc(threadsSize);
    if (tp->threads == NULL) {
        free(tp);
        sys_error();
    }

    // try to create threads
    int i;
    for (i = 0; i < threadNum; i++) {
        if (pthread_create(&(tp->threads[i]), NULL, exec, (void *) tp) != 0) {
            tpDestroy(tp, 0);
            sys_error();
        }
    }

    return tp;
}


// the function gets pointer to thread pool and shouldWaitForTasks
// it updates thread pool's state according to shouldWaitForTasks and frees all
void tpDestroy(ThreadPool *tp, int shouldWaitForTasks) {
    // case destroy called already
    if(tp->state != RUNNING) {
        return;
    }

    // lock thread pool mutex
    if(pthread_mutex_lock(&(tp->mutex)) != 0){
        sys_error(tp);
    }

    // case shouldWaitForTasks = 0 wait only to running tasks
    if (shouldWaitForTasks == 0) {
        // free tasks in queue
        while(!osIsQueueEmpty(tp->queue)){
            free(osDequeue(tp->queue));
        }
        tp->state = WAIT_RUNNING;
        // case shouldWaitForTasks != 0 wait to tasks in queue too
    } else {
        tp->state = WAIT_ALL;
    }

    // unlock thread pool mutex
    if(pthread_mutex_unlock(&(tp->mutex)) != 0){
        sys_error(tp);
    }

    // unblock all threads that block on condition
    if(pthread_cond_broadcast(&(tp->condition)) != 0){
        sys_error(tp);
    }

    // join all threads
    int i;
    for (i = 0; i < tp->threadNum; i++) {
        if(pthread_join(tp->threads[i], NULL) != 0){
            sys_error();
        }
    }

    // free all
    free(tp->threads);
    osDestroyQueue(tp->queue);

    if(pthread_mutex_destroy(&(tp->mutex)) != 0){
        sys_error();
    }
    if(pthread_cond_destroy(&(tp->condition)) != 0){
        sys_error();
    }

    free(tp);
}


// the function gets thread pool, func and args
// it inserts the func and args as task to the thread pool
int tpInsertTask(ThreadPool *tp, void (*computeFunc)(void *), void *args) {
    // case thread pool isn't running
    if (tp->state != RUNNING) {
        return -1;
    }

    // try to alloc task
    Task *task = (Task *) malloc(sizeof(Task));
    if (task == NULL) {
        sys_error();
    }

    // set task's func and param
    task->func = computeFunc;
    task->args = args;

    // lock thread pool's mutex
    if(pthread_mutex_lock(&(tp->mutex)) != 0){
        sys_error();
    }

    // insert task to queue
    osEnqueue(tp->queue, task);

    // signal that queue isn't empty
    if(pthread_cond_broadcast(&(tp->condition)) != 0){
        sys_error();
    }

    // unlock thread pool's mutex
    if(pthread_mutex_unlock(&(tp->mutex)) != 0){
        sys_error();
    }

    return 0;
}

