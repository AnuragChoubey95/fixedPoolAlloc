#include <stdint.h>
#include <utility>
#include <cstddef>
#include <iostream>
#include <atomic>

#define BLOCK_SIZE 64
#define NUM_BLOCKS 64

struct Heap {
    uint8_t pool_[NUM_BLOCKS * BLOCK_SIZE] = {0};
    std::atomic<uint64_t> metadata_{0}; 

    int claimFirstFreeIdx(){
        uint64_t bitField = metadata_.load(std::memory_order_relaxed);

        if (bitField == 0xFFFFFFFFFFFFFFFFULL) return -1;

        while (true){
            uint64_t inverted = ~bitField;
            if (inverted == 0) return -1;

            int bit = __builtin_ctzll(inverted);
            uint64_t mask = 1ULL << bit;

            uint64_t newBitField = bitField | mask;

            if (metadata_.compare_exchange_weak(bitField, newBitField, std::memory_order_acquire, std::memory_order_relaxed)) return bit;
        }
    }

    int releaseIdx(int idx){
        uint64_t bitField = metadata_.load(std::memory_order_relaxed);
        uint64_t mask = 1ULL << idx;

        if ((bitField & mask) == 0) return -1; 
        while(true){
            uint64_t newBitField = bitField & ~mask;

            if (metadata_.compare_exchange_weak(bitField, newBitField, std::memory_order_acquire, std::memory_order_relaxed)) return 0;

            if ((bitField & mask) == 0) return -1;
        }
    }
};


struct MemRange {
    uint8_t* lo = nullptr;
    uint8_t* hi = nullptr;
};


class FixedAllocator{
public:
    FixedAllocator(){}

    MemRange my_malloc(){
        int freeIdx = myHeap_.claimFirstFreeIdx();
        MemRange memBlock;
        
        if (freeIdx != -1){
            uint8_t* startMemAddr = &(myHeap_.pool_[freeIdx * BLOCK_SIZE]);
            uint8_t* endMemAddr = &(myHeap_.pool_[(freeIdx + 1) * BLOCK_SIZE - 1]);

            memBlock.lo = startMemAddr;
            memBlock.hi = endMemAddr;
        }
        return memBlock;
    }

    bool my_free(const MemRange memBlock){
        if ((memBlock.lo - &myHeap_.pool_[0]) % BLOCK_SIZE) return false;
        int idxToFree = (memBlock.lo - &myHeap_.pool_[0]) / BLOCK_SIZE;
        if(!(idxToFree < 64 && idxToFree >= 0)) return false;

        return myHeap_.releaseIdx(idxToFree) == 0;
    }

private:
    Heap myHeap_;
};

// Next: Think alignment