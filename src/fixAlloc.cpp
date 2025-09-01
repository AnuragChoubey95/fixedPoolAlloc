/* 
    Notes: 
    >   Why I chose cas_weak vs cas_strong
        https://devblogs.microsoft.com/oldnewthing/20180330-00/?p=98395
*/

#include "fixAlloc.h"
#include <iostream>

int Heap::claimFirstFreeIdx(){
    uint64_t bitField = metadata_.load();
    if (bitField == 0xFFFFFFFFFFFFFFFFULL) return -1;

    while (true){
        uint64_t inverted = ~bitField;
        if (inverted == 0) return -1;
        int bit = __builtin_ctzll(inverted);
        uint64_t mask = 1ULL << bit;
        uint64_t newBitField = bitField | mask;
        if (metadata_.compare_exchange_weak(bitField, newBitField)) return bit;
    }
}

int Heap::releaseIdx(int idx){
    uint64_t bitField = metadata_.load();
    uint64_t mask = 1ULL << idx;

    if ((bitField & mask) == 0) return -1;
    while(true){
        uint64_t newBitField = bitField & ~mask;
        if (metadata_.compare_exchange_weak(bitField, newBitField)) return 0;
        if ((bitField & mask) == 0) return -1;
    }
}

FixedAllocator::FixedAllocator() {}

MemRange FixedAllocator::my_malloc(){
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

bool FixedAllocator::my_free(const MemRange memBlock){
    if (!memBlock.lo || !memBlock.hi) return false;
    if ((memBlock.lo - &myHeap_.pool_[0]) % BLOCK_SIZE) return false;
    int idxToFree = (memBlock.lo - &myHeap_.pool_[0]) / BLOCK_SIZE;
    if(!(idxToFree < 64 && idxToFree >= 0)) return false;

    return myHeap_.releaseIdx(idxToFree) == 0;
}
