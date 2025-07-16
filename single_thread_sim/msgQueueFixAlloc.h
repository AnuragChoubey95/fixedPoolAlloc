#include <cstring>
#include "../fixAlloc.h"

#define QUEUE_MAX_SIZE NUM_BLOCKS

class MessageQueueFixAlloc{
    
public:
    MessageQueueFixAlloc() {
        head = tail = count = 0;
    }

    bool enqueue(const uint8_t* data) {
        if (count >= QUEUE_MAX_SIZE) return false;

        MemRange r = alloc_.my_malloc();
        if (!r.lo) return false; 

        memcpy(r.lo, data, BLOCK_SIZE);
        entries[tail] = r;

        tail = (tail + 1) % QUEUE_MAX_SIZE;
        ++count;
        return true;
    }

    bool dequeue(uint8_t* out_data) {
        if (count == 0) return false;

        MemRange r = entries[head];
        memcpy(out_data, r.lo, BLOCK_SIZE);
        bool freed = alloc_.my_free(r);
        if (!freed) return false; 

        head = (head + 1) % QUEUE_MAX_SIZE;
        --count;
        return true;
    }

    size_t size() const {
        return count;
    }


private:
    MemRange entries[QUEUE_MAX_SIZE];
    FixedAllocator alloc_;
    size_t head;
    size_t tail;
    uint16_t count;
};