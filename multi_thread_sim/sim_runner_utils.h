#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <cstring>

#include "../src/msgQueueFixAlloc.h"
#include "../src/msgQueueStd.h"
#include "../src/metrics.h"

#define MSG_PATTERN 0xAB

template <typename QueueType>
class SimRunnerMT {
public:
    SimRunnerMT(size_t producers, size_t consumers, size_t ticks);

    void run(const std::string& name);

private:
    void produce_loop(size_t producer_id, QueueType& queue, ThreadMetrics& tm);
    void consume_loop(size_t consumer_id, QueueType& queue, ThreadMetrics& tm);

    size_t random_msg_size(std::mt19937& rng);
    size_t random_hold_ticks(std::mt19937& rng);
    void idle_gap(std::mt19937& rng);

    size_t num_producers;
    size_t num_consumers;
    size_t total_ticks;

    std::atomic<size_t> sent;
    std::atomic<size_t> dropped;
    std::atomic<size_t> received;
};
