cmake_minimum_required(VERSION 3.9)
project(threadpool)
find_package(Threads)
set(CMAKE_CXX_STANDARD 17)

add_executable(threadpool threadpool.cpp )
target_link_libraries (threadpool ${CMAKE_THREAD_LIBS_INIT})
