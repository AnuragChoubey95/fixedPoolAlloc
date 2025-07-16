set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -fsanitize=address")

cmake_minimum_required(VERSION 3.15)
project(FixedAllocatorTests)

enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(allocator_tests
    tests/allocator_tests.cpp
    fixAlloc.h
)

target_link_libraries(allocator_tests
    GTest::gtest
    GTest::gtest_main
    pthread
)

add_test(NAME FixedAllocatorSuite COMMAND allocator_tests)

add_executable(queue_tests_fix_alloc
    tests/queue_tests_fix_alloc.cpp
    single_thread_sim/msgQueueFixAlloc.h
    fixAlloc.h
)

target_link_libraries(queue_tests_fix_alloc
    GTest::gtest
    GTest::gtest_main
    pthread
)

add_executable(sim_benchmark
    single_thread_sim/sim_benchmark.cpp
    single_thread_sim/msgQueueFixAlloc.h
    single_thread_sim/msgQueueStd.h
    fixAlloc.h
)

add_test(NAME MessageQueueSuite COMMAND queue_tests_fix_alloc)
