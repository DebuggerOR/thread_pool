cmake_minimum_required(VERSION 3.14)
project(thread_pool C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(thread_pool osqueue.c osqueue.h strange_test.c my_test.c threadPool.c)