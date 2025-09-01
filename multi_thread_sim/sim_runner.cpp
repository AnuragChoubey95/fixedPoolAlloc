#include "sim_runner_utils.h"

template <typename QueueType>
void SimRunnerMT<QueueType>::run(const std::string& name) {
    QueueType queue;
    Metrics global_metrics;

    auto start = std::chrono::steady_clock::now();

    std::vector<ThreadMetrics> thread_metrics(num_producers + num_consumers);
    std::vector<std::thread> threads;
    threads.reserve(num_producers + num_consumers);

    for (size_t p = 0; p < num_producers; ++p) {
        threads.emplace_back(&SimRunnerMT::produce_loop, this,
                             p, std::ref(queue), std::ref(thread_metrics[p]));
    }

    for (size_t c = 0; c < num_consumers; ++c) {
        threads.emplace_back(&SimRunnerMT::consume_loop, this,
                             c, std::ref(queue),
                             std::ref(thread_metrics[num_producers + c]));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    for (auto& tm : thread_metrics) {
        global_metrics.merge(tm);
    }

    std::cout << "[" << name << "]\n";
    global_metrics.summarize(std::cout);
    std::cout << "Duration: " << duration << "us\n\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <num_producers> <num_consumers> <ticks_per_thread>\n"
                  << "  <num_producers>   Number of producer threads (positive integer)\n"
                  << "  <num_consumers>   Number of consumer threads (positive integer)\n"
                  << "  <ticks_per_thread> Number of iterations per thread (positive integer)\n";
        return 1;
    }

    size_t producers, consumers, ticks;

    try {
        producers = std::stoul(argv[1]);
        consumers = std::stoul(argv[2]);
        ticks     = std::stoul(argv[3]);
    } catch (const std::invalid_argument&) {
        std::cerr << "Error: All arguments must be integers.\n";
        return 1;
    } catch (const std::out_of_range&) {
        std::cerr << "Error: Argument value out of range for unsigned long.\n";
        return 1;
    }

    if (producers == 0 || consumers == 0 || ticks == 0) {
        std::cerr << "Error: All arguments must be positive integers.\n";
        return 1;
    }

    std::cout << "Running with "
              << producers << " producers, "
              << consumers << " consumers, "
              << ticks << " ticks per thread.\n\n";

    SimRunnerMT<MessageQueueFixAlloc> sim_fix(producers, consumers, ticks);
    sim_fix.run("Fixed Allocator MT");

    SimRunnerMT<MessageQueueStd> sim_std(producers, consumers, ticks);
    sim_std.run("Std Allocator MT");

    return 0;
}

