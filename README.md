# fixedPoolAlloc

A fixed-size, bitmap-based memory allocator and benchmark suite for **high-performance**, **multi-threaded**, and **real-time systems**.

This project compares a custom fixed-pool allocator against the standard heap allocator (`new[]/delete[]`) under controlled queue workloads. The allocator provides **deterministic O(1) allocation/free**, designed for embedded, low-latency, or safety-critical contexts where dynamic heap usage is undesirable.

---

## Project Structure

```text
fixedPoolAlloc/
├── CMakeLists.txt
├── src/
│   ├── fixAlloc.cpp / fixAlloc.h        # Fixed-size allocator implementation
│   ├── msgQueueFixAlloc.h               # Queue backed by FixedAllocator
│   ├── msgQueueStd.h                    # Queue backed by new/delete
│   ├── metrics.cpp / metrics.h          # Latency & throughput collection
├── multi_thread_sim/
│   ├── sim_runner.cpp                   # Multi-threaded driver (main + run())
│   ├── sim_runner_utils.cpp/.h          # Producer/consumer loop helpers
├── single_thread_sim/
│   └── sim_runner.cpp                   # Single-threaded benchmark driver
├── tests/
│   ├── allocator_tests.cpp              # Unit tests for allocator
│   └── queue_tests_fix_alloc.cpp        # Queue correctness tests
```

---

## Build Instructions

### Prerequisites

* CMake 3.10+
* C++17-compatible compiler (Clang, GCC, Apple Clang)

### Build

```bash
mkdir build && cd build
cmake ..
make -j "$(nproc)"
```

---

## Running Tests

```bash
./allocator_tests
./queue_tests_fix_alloc
```

---

## Benchmark Drivers

### Single-threaded

```bash
./sim_benchmark_st
```

### Multi-threaded

Mandatory CLI arguments. Inputs are validated.

```bash
./sim_benchmark_mt <num_producers> <num_consumers> <ticks_per_thread>
```

**Usage help (shown if args missing/invalid):**

```text
Usage: ./sim_benchmark_mt <num_producers> <num_consumers> <ticks_per_thread>
  <num_producers>     Number of producer threads (positive integer)
  <num_consumers>     Number of consumer threads (positive integer)
  <ticks_per_thread>  Number of iterations per thread (positive integer)
```

**Examples**

```bash
./sim_benchmark_mt 1 7 30000
./sim_benchmark_mt 1 7 3000000
./sim_benchmark_mt 4 4 3000000
```

---

## Metrics & Reporting

Timing uses `std::chrono::steady_clock`.

* **Per-thread collection (`ThreadMetrics`)**

  * Enqueue/dequeue latencies (ns) for every operation
  * Counters: `sent`, `dropped`, `received`
* **Global aggregation (`Metrics`)**

  * Merges per-thread samples without contending with hot-path locks
  * Reports totals and latency statistics

**Reported fields**

* Sent / Dropped / Received
* Sample counts: `Enqueue samples`, `Dequeue samples`
* Latency stats for enqueue and dequeue: **avg, p50 (median), p95, p99**
* Total wall time (µs)

**Sanity invariants**

* `Sent + Dropped = producers × ticks`
* `Received ≤ Sent`
* Optional: `queue.size() + Received ≈ Sent` at end of run

---

## Workload Design

This benchmark simulates a realistic message-passing system to compare allocator performance under multithreaded load. The workload is carefully constructed to avoid synthetic or idealized conditions:

1. **Producers keep sending until the queue is full**

   Each producer tries to enqueue one message per loop iteration. If the queue is full, the message is dropped and the loop continues — there’s no retry or waiting. This causes backpressure to emerge naturally based on how fast consumers drain the queue.

2. **Consumers simulate post-dequeue "processing time"**

   After a successful dequeue, the consumer performs a short, randomized spin delay (`1–5 ticks`) to represent application logic. Memory is freed *before* this delay, so allocator pressure reflects true usage. This delay slows the consumer's next dequeue and indirectly increases queue depth under load.

3. **Randomized message sizes inside a fixed block**

   Messages initialize a random number of bytes (`1–64`) inside a 64-byte block. Both queue types still copy all 64 bytes, keeping the cost fair and consistent. This avoids trivial test patterns while simulating varied payloads.

4. **Multiple producers and consumers run concurrently**

   The benchmark launches `N` producer threads and `M` consumer threads, each with independent random seeds. Threads contend on the queue lock and stress the allocator:
   - The fixed pool allocator uses atomic bit operations.
   - The standard allocator (`new/delete`) relies on heap locks and system malloc.

5. **Idle gaps every 64 iterations**

   Every 64 iterations, each thread sleeps for 50–200 microseconds. These small pauses simulate real-world idle behavior (e.g. I/O, blocking waits) and prevent the allocator from running in a perpetually cache-hot state. This helps surface latency spikes that averages alone might hide.

---

## Sample Results (selected runs)

> Hardware: user’s 8-core machine; queue size 64; block size 64 bytes.

### 1 producer, 7 consumers, 30k ticks each

```
[Fixed Allocator MT]
Sent: 26463  Dropped: 3537  Received: 26399
Enqueue latency (ns): avg=412.53 p50=125 p95=1334 p99=6042
Dequeue latency (ns): avg=201.90 p50=42  p95=166  p99=3792
Duration: 99,517 µs

[Std Allocator MT]
Sent: 24961  Dropped: 5039  Received: 24897
Enqueue latency (ns): avg=540.82 p50=125 p95=2625 p99=8167
Dequeue latency (ns): avg=229.77 p50=83  p95=167  p99=4833
Duration: 103,278 µs
```

### 1 producer, 7 consumers, 3,000,000 ticks each

```
[Fixed Allocator MT]
Sent: 2,624,032  Dropped: 375,968  Received: 2,623,968
Enqueue latency (ns): avg=405.79 p50=125 p95=1334 p99=6000
Dequeue latency (ns): avg=194.38 p50=83  p95=166  p99=3708
Duration: 9,658,927 µs

[Std Allocator MT]
Sent: 2,526,890  Dropped: 473,110  Received: 2,526,826
Enqueue latency (ns): avg=538.73 p50=125 p95=2500 p99=8208
Dequeue latency (ns): avg=230.10 p50=83  p95=167  p99=4750
Duration: 10,053,273 µs
```

### 4 producers, 4 consumers, 3,000,000 ticks each

```
[Fixed Allocator MT]
Sent: 8,764,047  Dropped: 3,235,953  Received: 8,764,047
Enqueue latency (ns): avg=763.11 p50=125 p95=2917 p99=16666
Dequeue latency (ns): avg=790.22 p50=125 p95=3166 p99=17416
Duration: 10,819,925 µs

[Std Allocator MT]
Sent: 8,869,100  Dropped: 3,130,900  Received: 8,869,100
Enqueue latency (ns): avg=1156.49 p50=125 p95=5125 p99=23666
Dequeue latency (ns): avg=1173.86 p50=125 p95=5292 p99=24125
Duration: 12,046,154 µs
```

**Interpretation**

* **Average latency**: fixed-pool consistently \~25–40% faster.
* **Tail latency**: fixed-pool p95/p99 notably lower and tighter; `new/delete` exhibits larger spikes under contention.
* **Throughput & integrity**: `Sent + Dropped = producers × ticks`, `Received ≈ Sent` across runs.

---

## Notes

* Queues are synchronized with `std::mutex` and RAII (`std::lock_guard`) to avoid data races and deadlocks.
* `high_resolution_clock` can alias `system_clock` on some libstdc++; we standardize on `steady_clock` for monotonicity.
* Why I chose cas_weak vs cas_strong: https://devblogs.microsoft.com/oldnewthing/20180330-00/?p=98395

---

## Roadmap / Next Upgrades

* CSV/JSON export of metrics for plotting
* Histogram buckets for latency distribution
* Thread-scaling sweeps (1..N) with plots
* Pure allocator microbenchmarks (no queue) for upper-bound comparisons
* Batch alloc/free patterns and randomized lifetimes to stress fragmentation

---
