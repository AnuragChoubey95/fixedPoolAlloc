#include "metrics.h"
#include <numeric>
#include <algorithm>
#include <iomanip>

void Metrics::merge(ThreadMetrics& tm) {
    std::lock_guard<std::mutex> lock(mtx);

    all_enqueue_latencies.insert(all_enqueue_latencies.end(),
                                 tm.enqueue_latencies.begin(),
                                 tm.enqueue_latencies.end());

    all_dequeue_latencies.insert(all_dequeue_latencies.end(),
                                 tm.dequeue_latencies.begin(),
                                 tm.dequeue_latencies.end());

    total_sent     += tm.sent;
    total_dropped  += tm.dropped;
    total_received += tm.received;
}

static void report_stats(const std::vector<long>& v,
                         const char* label,
                         std::ostream& out) {
    if (v.empty()) return;

    std::vector<long> sorted = v;
    std::sort(sorted.begin(), sorted.end());

    auto avg = std::accumulate(sorted.begin(), sorted.end(), 0LL) /
               static_cast<double>(sorted.size());

    auto p50 = sorted[sorted.size() * 50 / 100];
    auto p95 = sorted[sorted.size() * 95 / 100];
    auto p99 = sorted[sorted.size() * 99 / 100];

    out << label << " latency (ns): "
        << "avg=" << std::fixed << std::setprecision(2) << avg
        << " p50=" << p50
        << " p95=" << p95
        << " p99=" << p99 << "\n";
}

void Metrics::summarize(std::ostream& out) {
    std::lock_guard<std::mutex> lock(mtx);

    out << "Sent:     "   << total_sent
        << "\nDropped:  " << total_dropped
        << "\nReceived: " << total_received
        << "\nEnqueue samples: " << all_enqueue_latencies.size()
        << "\nDequeue samples: " << all_dequeue_latencies.size()
        << "\n";

    report_stats(all_enqueue_latencies, "Enqueue", out);
    report_stats(all_dequeue_latencies, "Dequeue", out);
}
