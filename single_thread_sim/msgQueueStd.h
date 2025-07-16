#include <cstdint>
#include <cstring>
#include <new>

#define BLOCK_SIZE 64
#define QUEUE_MAX_SIZE 64

class MessageQueueStd {
public:
    MessageQueueStd() : head(0), tail(0), count(0) {}

    bool enqueue(const uint8_t* data) {
        if (count >= QUEUE_MAX_SIZE) return false;

        uint8_t* block = new (std::nothrow) uint8_t[BLOCK_SIZE];
        if (!block) return false;

        memcpy(block, data, BLOCK_SIZE);
        entries[tail] = block;

        tail = (tail + 1) % QUEUE_MAX_SIZE;
        ++count;
        return true;
    }

    bool dequeue(uint8_t* out_data) {
        if (count == 0) return false;

        uint8_t* block = entries[head];
        memcpy(out_data, block, BLOCK_SIZE);
        delete[] block;

        head = (head + 1) % QUEUE_MAX_SIZE;
        --count;
        return true;
    }

    size_t size() const {
        return count;
    }

private:
    uint8_t* entries[QUEUE_MAX_SIZE];
    size_t head;
    size_t tail;
    uint16_t count;
};
