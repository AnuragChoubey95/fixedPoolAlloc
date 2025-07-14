# fixedPoolAlloc

🚧 Coming Soon
- Thread-Safe Allocations: lock-free and atomics-based synchronization for safe concurrent access.
- Hardware-Aware Pool Layout: memory pool layout optimized for cacheline alignment and reduced contention on modern CPU architectures.

A fixed-size memory allocator and message queue benchmark suite for real-time systems.
Designed to test performance-critical memory management patterns in embedded and
low-latency applications.
## Project Structure
```
fixedPoolAlloc/
fixAlloc.h msgQueueFixAlloc.h msgQueueStd.h sim_benchmark.cpp 

  tests/
    allocator_tests.cpp # GTest suite for FixedAllocator
    queue_tests_fix_alloc.cpp # GTest suite for message queue

  .gitignore 
  CMakeLists.txt # Build system using CMake 3.10+
  README.md # You're reading this
```
## Build Instructions
### Prerequisites
- CMake 3.10+
- A C++ compiler (GCC, Clang, or Apple Clang)
### Build
```bash
git clone https://github.com/AnuragChoubey95/fixedPoolAlloc.git
cd fixedPoolAlloc
mkdir build && cd build
cmake ..
make -j
```
## Running Tests
```bash
./allocator_tests
./queue_tests_fix_alloc
```
## Running Benchmarks
```bash
./sim_benchmark
```
Sample output:
```
[Fixed Allocator]
Sent: 100000
Dropped: 0
Received: 100000
Final Q size: 0
Duration: 17xxx us
[Std Allocator]
Sent: 100000
Dropped: 0
Received: 100000
Final Q size: 0
Duration: 40xxx us
```
## Module Breakdown
### fixAlloc.h
- Fixed-size memory allocator with a bitmap metadata pool.
- O(1) allocation and deallocation.
- No dynamic allocation, thread-safe behavior not guaranteed.
### msgQueueFixAlloc.h
- Lock-free circular queue using fixed allocator.
- Suitable for real-time systems where malloc is forbidden.
### msgQueueStd.h
- Identical API but uses heap (new/delete).
- For baseline performance comparison only.
### sim_benchmark.cpp
- Measures enqueue/dequeue throughput under high volume.
- Times both allocators for fair comparison.
## Test Coverage
### Allocator Tests
- Exhaustive allocation and reuse.
fixedPoolAlloc README
- Double frees.
- Invalid or misaligned frees.
- Memory integrity after reuse.
- Index-free pointer-based validation.
### Message Queue Tests
- FIFO order preservation.
- Full/empty detection.
- Stress enqueue/dequeue loops.
