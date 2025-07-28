//Author: Anurag Choubey
#pragma once
#include <stdint.h>
#include <utility>
#include <cstddef>
#include <atomic>

#define BLOCK_SIZE 64
#define NUM_BLOCKS 64

struct Heap {
    uint8_t pool_[NUM_BLOCKS * BLOCK_SIZE] = {0};
    std::atomic<uint64_t> metadata_{0};

    int claimFirstFreeIdx();
    int releaseIdx(int idx);
};

struct MemRange {
    uint8_t* lo = nullptr;
    uint8_t* hi = nullptr;
};

class FixedAllocator{
public:
    FixedAllocator();
    MemRange my_malloc();
    bool my_free(const MemRange memBlock);

private:
    Heap myHeap_;
};
