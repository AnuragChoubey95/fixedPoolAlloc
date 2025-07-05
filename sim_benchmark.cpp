#include <iostream>
#include <chrono>
#include <cstring>
#include "msgQueueFixAlloc.h"
#include "msgQueueStd.h"

#define TICKS 100000
#define MSG_PATTERN 0xAB

class SimRunner {
public:
    template <typename QueueType>
    static void run(const std::string& name) {
        QueueType queue;
        size_t sent = 0, dropped = 0, received = 0;
        uint8_t buffer[BLOCK_SIZE];
        uint8_t out[BLOCK_SIZE];

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < TICKS; ++i) {
            memset(buffer, MSG_PATTERN, BLOCK_SIZE);
            if (queue.enqueue(buffer)) ++sent;
            else ++dropped;

            if (queue.dequeue(out)) ++received;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << "[" << name << "]\n"
                  << "Sent:     " << sent << "\n"
                  << "Dropped:  " << dropped << "\n"
                  << "Received: " << received << "\n"
                  << "Final Q size: " << queue.size() << "\n"
                  << "Duration: " << duration << "us\n\n";
    }
};

int main() {
    SimRunner::run<MessageQueueFixAlloc>("Fixed Allocator");
    SimRunner::run<MessageQueueStd>("Std Allocator");
    return 0;
}
