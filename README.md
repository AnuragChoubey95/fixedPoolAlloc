# fixedPoolAlloc

A fixed-size, bitmap-based memory allocator and message queue benchmark suite designed for **high-performance**, **multi-threaded**, and **real-time systems**.

This project focuses on **zero-dynamic-allocation memory management**, offering fast, deterministic `O(1)` allocations suitable for embedded, low-latency, or safety-critical applications.

---

## Project Structure

```markdown
fixedPoolAlloc/
├── fixAlloc.h               # Fixed-size memory allocator with atomic metadata
├── single_thread_sim/
│   ├── msgQueueFixAlloc.h   # Fixed allocator queue (single-threaded)
│   ├── msgQueueStd.h        # Standard allocator queue (for comparison)
│   └── sim_benchmark.cpp    # Single-threaded benchmark driver
├── tests/
│   ├── allocator_tests.cpp           # GoogleTest suite for Fixed Allocator
│   └── queue_tests_fix_alloc.cpp     # GoogleTest suite for message queue
├── CMakeLists.txt
└── README.md
```

---

## ⚙Build Instructions

### Prerequisites

* CMake 3.10+
* C++17 compatible compiler (GCC, Clang, Apple Clang)

### Build

```bash
git clone https://github.com/AnuragChoubey95/fixedPoolAlloc.git
cd fixedPoolAlloc
mkdir build && cd build
cmake ..
make -j `nproc`
```

---

## Running Tests

Unit tests for allocator correctness and queue FIFO behavior:

```bash
./allocator_tests
./queue_tests_fix_alloc
```

---

## Running Benchmarks

### Single-threaded Throughput Simulation:

```bash
./sim_benchmark
```

Sample output:

```plaintext
[Fixed Allocator]
Sent: 100000
Dropped: 0
Received: 100000
Final Q size: 0
Duration: 17xx us

[Std Allocator]
Sent: 100000
Dropped: 0
Received: 100000
Final Q size: 0
Duration: 20xx us
```

### Multi-threaded Benchmark:

Coming soon: `sim_benchmark_mt` — benchmarks the atomic, lock-free allocator performance under thread contention.

---

## Module Breakdown

### `fixAlloc.h`

* Fixed-size memory allocator with **atomic bitfield metadata**.
* Fast `claimFirstFreeIdx()` and `releaseIdx()` using `compare_exchange_weak`.
* **Thread-safe** with relaxed memory ordering optimizations.
* Designed for **zero dynamic allocations**, **constant-time** operations.

### `msgQueueFixAlloc.h`

* Circular queue using `fixAlloc.h`, zero heap allocations.
* Focused on low-latency single-threaded scenarios.

### `msgQueueStd.h`

* Same API as `msgQueueFixAlloc.h` but uses standard `new/delete` allocation.
* Serves as a **performance baseline**.

### `sim_benchmark.cpp`

* Simulates single-threaded enqueue/dequeue throughput at scale.
* Compares fixed allocator vs standard allocator.

---

## Test Coverage

### Allocator Tests

* Exhaustive allocation/free cycles.
* Invalid free detection (double frees, out-of-bound indices).
* Memory reuse and integrity under stress.
* Index-free pointer conversion checks.

### Message Queue Tests

* FIFO order guarantee.
* Full/empty queue detection.
* High-volume enqueue/dequeue loops.

---
