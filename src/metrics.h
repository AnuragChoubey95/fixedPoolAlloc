#pragma once

#include <vector>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include <ostream>

// Per-thread collector
struct ThreadMetrics {
    std::vector<long> enqueue_latencies;
    std::vector<long> dequeue_latencies;

    size_t sent     = 0;
    size_t dropped  = 0;
    size_t received = 0;

    void record_enqueue(long ns, bool success) {
        enqueue_latencies.push_back(ns);
        if (success) ++sent;
        else ++dropped;
    }

    void record_dequeue(long ns, bool success) {
        dequeue_latencies.push_back(ns);
        if (success) ++received;
    }
};

// Global aggregator
class Metrics {
public:
    void merge(ThreadMetrics& tm);
    void summarize(std::ostream& out);

private:
    mutable std::mutex mtx;

    std::vector<long> all_enqueue_latencies;
    std::vector<long> all_dequeue_latencies;

    size_t total_sent     = 0;
    size_t total_dropped  = 0;
    size_t total_received = 0;
};
