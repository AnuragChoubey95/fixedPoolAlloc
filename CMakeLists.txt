cmake_minimum_required(VERSION 3.10)
project(FixedAllocatorTests)

# Enable address sanitizer and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -fsanitize=address")

# GoogleTest via FetchContent
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)

# Only needed for MSVC but harmless to keep
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# === Test target: allocator_tests ===
add_executable(allocator_tests
    tests/allocator_tests.cpp
    src/fixAlloc.cpp
)
target_link_libraries(allocator_tests
    gtest
    gtest_main
    pthread
)
add_test(NAME FixedAllocatorSuite COMMAND allocator_tests)

# === Test target: queue_tests_fix_alloc ===
add_executable(queue_tests_fix_alloc
    tests/queue_tests_fix_alloc.cpp
    src/fixAlloc.cpp
)
target_link_libraries(queue_tests_fix_alloc
    gtest
    gtest_main
    pthread
)
add_test(NAME MessageQueueSuite COMMAND queue_tests_fix_alloc)

# === Benchmark: sim_benchmark ===
add_executable(sim_benchmark
    single_thread_sim/sim_benchmark.cpp
    src/fixAlloc.cpp
)
