
#include "threadPool.h"


// the function displays error message and exits
void sys_error() {
    write(2, "Error in system call\n", strlen("Error in system call\n"));
    exit(-1);
}


// the function gets thread pool as void
// it does the tasks while tp running or waiting to tasks in queue
static void *exec(void *x) {
    // try to convert to thread pool
    ThreadPool *tp = (ThreadPool *) x;
    if (tp == NULL) {
        sys_error();
    }

    // loop's conditions
    int isRunning = (tp->state == ONLINE);
    int isOffline = (tp->state == OFFLINE);
    int isQueueNotEmpty = !(osIsQueueEmpty(tp->queue));

    // while tp running or tp waits to all and queue isn't empty
    while (isRunning || (isOffline && isQueueNotEmpty)) {
        // lock thread pool's mutex
        if (pthread_mutex_lock(&(tp->mutex)) != 0) {
            sys_error();
        }

        // block on condition
        while (tp->state == ONLINE && osIsQueueEmpty(tp->queue)) {
            if (pthread_cond_wait(&(tp->condition), &(tp->mutex)) != 0) {
                sys_error();
            }
        }

        // dequeue task from tasks' queue
        Task *task = (Task *) osDequeue(tp->queue);

        // lock thread pool's mutex
        if (pthread_mutex_unlock(&(tp->mutex)) != 0) {
            sys_error();
        }

        // do task
        if (task != NULL) {
            ((task->func))(task->args);
            free(task);
        }

        // update loop condition vars
        isRunning = (tp->state == ONLINE);
        isOffline = (tp->state == OFFLINE);
        isQueueNotEmpty = !(osIsQueueEmpty(tp->queue));
    }

    pthread_exit(NULL);
}


// the function gets num of threads
// it creates and returns a thread pull with this num of threads
ThreadPool *tpCreate(int threadNum) {
    // case no positive num threads
    if (threadNum < 1) {
        return NULL;
    }

    // else try to create thread pool
    ThreadPool *tp = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (tp == NULL) {
        sys_error();
    }

    // set thread pool's fields
    tp->state = ONLINE;
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
    int threadsSize = sizeof(pthread_t) * (size_t) threadNum;
    tp->threads = (pthread_t *) malloc(threadsSize);
    if (tp->threads == NULL) {
        free(tp);
        sys_error();
    }

    // try to create threads
    int i;
    for (i = 0; i < threadNum; ++i) {
        if (pthread_create(&(tp->threads[i]), NULL, exec, (void *) tp) != 0) {
            tpDestroy(tp, 0);
            sys_error();
        }
    }

    return tp;
}


// the function gets thread pool, func and args
// it inserts the func and args as task to the thread pool
int tpInsertTask(ThreadPool *tp, void (*computeFunc)(void *), void *args) {
    // case thread pool isn't running
    if (tp->state != ONLINE) {
        return -1;
    }

    // try to alloc task
    Task *task = (Task *) malloc(sizeof(Task));
    if (task == NULL) {
        sys_error();
    }

    // set task's func and args
    task->args = args;
    task->func = computeFunc;

    // lock thread pool's mutex
    if (pthread_mutex_lock(&(tp->mutex)) != 0) {
        sys_error();
    }

    // insert task to queue
    osEnqueue(tp->queue, task);

    // signal that queue isn't empty
    if (pthread_cond_broadcast(&(tp->condition)) != 0) {
        sys_error();
    }

    // unlock thread pool's mutex
    if (pthread_mutex_unlock(&(tp->mutex)) != 0) {
        sys_error();
    }

    return 0;
}


// the function gets pointer to thread pool and shouldWaitForTasks
// it updates thread pool's state according to shouldWaitForTasks and frees all
void tpDestroy(ThreadPool *tp, int shouldWaitForTasks) {
    // case destroy called already
    if (tp->state == OFFLINE) {
        return;
    }

    // lock thread pool mutex
    if (pthread_mutex_lock(&(tp->mutex)) != 0) {
        sys_error();
    }

    // if should wait for tasks is 0 free all tasks
    if (shouldWaitForTasks == 0) {
        while (!osIsQueueEmpty(tp->queue)) {
            free(osDequeue(tp->queue));
        }
    }

    // update thread pool's state
    tp->state = OFFLINE;

    // unlock thread pool mutex
    if (pthread_mutex_unlock(&(tp->mutex)) != 0) {
        sys_error();
    }

    // unblock all threads that block on condition
    if (pthread_cond_broadcast(&(tp->condition)) != 0) {
        sys_error();
    }

    // join all threads
    int i;
    for (i = 0; i < tp->threadNum; ++i) {
        if (pthread_join(tp->threads[i], NULL) != 0) {
            sys_error();
        }
    }

    // free all
    free(tp->threads);
    osDestroyQueue(tp->queue);

    if (pthread_mutex_destroy(&(tp->mutex)) != 0) {
        sys_error();
    }
    if (pthread_cond_destroy(&(tp->condition)) != 0) {
        sys_error();
    }

    free(tp);
}
