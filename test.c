

#include <stdio.h>
#include <unistd.h>
#include "threadPool.h"
#include "osqueue.h"


void stupid_task(void *a) {
    printf("\n\t1 + 1 = 3\n");
}

void test_thread_pool_sanity(int numThreads) {
    ThreadPool *tp = tpCreate(numThreads);

    int i;
    for (i = 0; i < numThreads; ++i) {
        tpInsertTask(tp, stupid_task, NULL);
    }

    tpDestroy(tp, 0);
}


// test my thread pool
int main() {
    // arg is num of threads
    test_thread_pool_sanity(5);

    return 0;
}
