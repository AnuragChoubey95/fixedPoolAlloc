#include <stdint.h>
#include <utility>
#include <cstddef>
#include <iostream>
#include <cassert>

#define BLOCK_SIZE 64
#define NUM_BLOCKS 64

typedef struct {
    uint8_t pool_[NUM_BLOCKS * BLOCK_SIZE] = {0};
    uint8_t metadata_[8] = {0};

    int getFirstFreeIdx(){
        for (uint8_t i = 0; i < 64; ++i){
            size_t byte = i / 8;
            size_t bit = i % 8;
            if (!((metadata_[byte] >> bit) & 1)){
                return (int)i;
            }
        }
        return -1;
    }

    bool getBit(int idx){
        size_t byte = idx / 8;
        size_t bit = idx % 8;

        return (metadata_[byte] >> bit) & 1;
    }

    void setBit(int idx, bool value){
        size_t byte = idx / 8;
        size_t bit = idx % 8;

        if (value){
            metadata_[byte] |= (1 << bit);
        } else {
            metadata_[byte] &= ~(1 << bit);
        }

    }
} Heap;


typedef struct {
    uint8_t* lo = nullptr;
    uint8_t* hi = nullptr;
} MemRange;


class FixedAllocator{
public:
    FixedAllocator(){}

    MemRange my_malloc(){
        MemRange memBlock;
        int freeIdx = myHeap_.getFirstFreeIdx();

        if (freeIdx != -1){
            uint8_t* startMemAddr = &(myHeap_.pool_[freeIdx * BLOCK_SIZE]);
            uint8_t* endMemAddr = &(myHeap_.pool_[(freeIdx + 1) * BLOCK_SIZE - 1]);

            memBlock.lo = startMemAddr;
            memBlock.hi = endMemAddr;

            myHeap_.setBit(freeIdx, true);
        }
        return memBlock;
    }

    bool my_free(const MemRange memBlock){
        if ((memBlock.lo - &myHeap_.pool_[0]) % BLOCK_SIZE) return false;
        int idxToFree = (memBlock.lo - &myHeap_.pool_[0]) / BLOCK_SIZE;
        if(!(idxToFree < 64 && idxToFree >= 0)) return false;
        if (!myHeap_.getBit(idxToFree)) return false;
        myHeap_.setBit(idxToFree, false);

        return true;
    }

private:
    Heap myHeap_;
};
