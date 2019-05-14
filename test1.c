//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include "osqueue.h"
//#include "threadPool.h"
//
//int i=0;
//
//void hello1 (void* a)
//{
//    printf("hello %d\n", i);
//    ++i;
//    //sleep(1) ;
//}
//
//
//void test_thread_pool_sanity()
//{
//    int i;
//
//    ThreadPool* tp = tpCreate(5);
//
//    for(i=0; i<50; ++i)
//    {
//        tpInsertTask(tp,hello1,NULL);
//    }
//    //sleep(1) ;
//    tpDestroy(tp,0);
//}
//
//
//int main()
//{
//    test_thread_pool_sanity();
//
//    return 0;
//}