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

// Print summary stats
void Metrics::summarize(std::ostream& out) {
    std::lock_guard<std::mutex> lock(mtx);

    out << "Sent:     "   << total_sent
        << "\nDropped:  " << total_dropped
        << "\nReceived: " << total_received
        << "\nEnqueue samples: " << all_enqueue_latencies.size()
        << "\nDequeue samples: " << all_dequeue_latencies.size()
        << "\n";

  
    if (!all_enqueue_latencies.empty()) {
        auto sum = std::accumulate(all_enqueue_latencies.begin(),
                                   all_enqueue_latencies.end(), 0LL);
        double avg = static_cast<double>(sum) / all_enqueue_latencies.size();
        out << "Average enqueue latency: " << std::fixed << std::setprecision(2) << avg << " ns\n";
    }

    if (!all_dequeue_latencies.empty()) {
        auto sum = std::accumulate(all_dequeue_latencies.begin(),
                                   all_dequeue_latencies.end(), 0LL);
        double avg = static_cast<double>(sum) / all_dequeue_latencies.size();
        out << "Average dequeue latency: " << std::fixed << std::setprecision(2) << avg << " ns\n";
    }
}
