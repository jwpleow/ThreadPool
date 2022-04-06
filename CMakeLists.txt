cmake_minimum_required(VERSION 3.15)
include_guard()
project(ThreadPool)

find_package(Threads REQUIRED)

add_library(ThreadPool INTERFACE)
add_library(ThreadPool::ThreadPool ALIAS ThreadPool)

target_link_libraries(ThreadPool INTERFACE Threads::Threads)

target_include_directories(ThreadPool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(ThreadPool INTERFACE cxx_std_17)


# add_executable(example example.cpp)
# target_link_libraries(example ThreadPool::ThreadPool)
