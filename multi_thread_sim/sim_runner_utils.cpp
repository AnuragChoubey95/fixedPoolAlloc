#include "sim_runner_utils.h"

template <typename QueueType>
SimRunnerMT<QueueType>::SimRunnerMT(size_t producers, size_t consumers, size_t ticks)
    : num_producers(producers),
      num_consumers(consumers),
      total_ticks(ticks),
      sent(0),
      dropped(0),
      received(0) {}

template <typename QueueType>
void SimRunnerMT<QueueType>::produce_loop(size_t producer_id, QueueType& queue, ThreadMetrics& tm) {
    std::mt19937 rng(producer_id + 1);
    std::uniform_int_distribution<int> holdDist(1, 10);
    std::uniform_int_distribution<int> idleDist(50, 200); 
    uint8_t buffer[BLOCK_SIZE];

    for (size_t i = 0; i < total_ticks; ++i) {
        size_t live_bytes = random_msg_size(rng);
        memset(buffer, MSG_PATTERN, live_bytes);

        auto t0 = std::chrono::steady_clock::now();
        bool ok = queue.enqueue(buffer);
        auto t1 = std::chrono::steady_clock::now();

        long dur = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        tm.record_enqueue(dur, ok);

        if (i % 64 == 0) {
            idle_gap(rng);
        }
    }
}

template <typename QueueType>
void SimRunnerMT<QueueType>::consume_loop(size_t consumer_id, QueueType& queue, ThreadMetrics& tm) {
    std::mt19937 rng(consumer_id + 101);
    std::uniform_int_distribution<int> holdDist(1, 10);  
    std::uniform_int_distribution<int> idleDist(50, 200); 

    uint8_t out[BLOCK_SIZE];

    for (size_t i = 0; i < total_ticks; ++i) {
        auto t0 = std::chrono::steady_clock::now();
        bool ok = queue.dequeue(out);
        auto t1 = std::chrono::steady_clock::now();

        long dur = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        tm.record_dequeue(dur, ok);

        if (ok) {
            int hold_ticks = random_hold_ticks(rng);
            if (hold_ticks > 1) {
                for (int h = 0; h < hold_ticks; ++h) {
                    // spin to simulate delayed free
                }
            }
        }
        if (i % 64 == 0) {
            idle_gap(rng);
        }
    }
}

template <typename QueueType>
size_t SimRunnerMT<QueueType>::random_msg_size(std::mt19937& rng) {
    std::uniform_int_distribution<size_t> dist(1, BLOCK_SIZE);
    return dist(rng);
}

template <typename QueueType>
size_t SimRunnerMT<QueueType>::random_hold_ticks(std::mt19937& rng) {
    std::uniform_int_distribution<size_t> dist(1, 5);
    return dist(rng);
}

template <typename QueueType>
void SimRunnerMT<QueueType>::idle_gap(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(50, 200);
    std::this_thread::sleep_for(std::chrono::microseconds(dist(rng)));
}

// Explicit instantiations so linker sees the symbols
template class SimRunnerMT<MessageQueueFixAlloc>;
template class SimRunnerMT<MessageQueueStd>;


