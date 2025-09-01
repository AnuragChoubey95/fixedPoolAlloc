#include <gtest/gtest.h>
#include <set>
#include <cstring>
#include <algorithm>
#include <random>
#include <thread>
#include "../src/fixAlloc.h"

#define NUM_CORES (std::thread::hardware_concurrency())

TEST(FixedAllocatorTest, ExhaustiveAllocation) {
    FixedAllocator allocator;
    MemRange blocks[NUM_BLOCKS];
    std::set<void*> seen;

    int count = 0;
    for (int i = 0; i < NUM_BLOCKS + 5; ++i) {
        MemRange r = allocator.my_malloc();
        if (r.lo && r.hi) {
            blocks[count++] = r;
            EXPECT_EQ(r.hi - r.lo + 1, BLOCK_SIZE);
            EXPECT_TRUE(seen.insert(r.lo).second);
        } else {
            break;
        }
    }
    EXPECT_EQ(count, NUM_BLOCKS);
}

TEST(FixedAllocatorTest, DoubleFreeDetection) {
    FixedAllocator allocator;
    MemRange r = allocator.my_malloc();
    ASSERT_TRUE(r.lo && r.hi);
    EXPECT_TRUE(allocator.my_free(r));
    EXPECT_FALSE(allocator.my_free(r));
}

TEST(FixedAllocatorTest, InvalidFree) {
    FixedAllocator allocator;
    MemRange fake;
    fake.lo = nullptr;
    fake.hi = nullptr;
    EXPECT_FALSE(allocator.my_free(fake));
}

TEST(FixedAllocatorTest, InterleavedAllocFree) {
    FixedAllocator allocator;
    MemRange a = allocator.my_malloc();
    MemRange b = allocator.my_malloc();
    MemRange c = allocator.my_malloc();

    EXPECT_TRUE(allocator.my_free(b));
    EXPECT_TRUE(allocator.my_free(a));

    MemRange d = allocator.my_malloc();
    MemRange e = allocator.my_malloc();

    EXPECT_TRUE(d.lo != nullptr);
    EXPECT_TRUE(e.lo != nullptr);
    std::set<void*> pointers = {d.lo, e.lo, c.lo};
    EXPECT_EQ(pointers.size(), 3);
}

TEST(FixedAllocatorTest, MemoryIntegrity) {
    FixedAllocator allocator;
    MemRange r = allocator.my_malloc();
    ASSERT_TRUE(r.lo);

    memset(r.lo, 0xAB, BLOCK_SIZE);
    EXPECT_TRUE(allocator.my_free(r));

    MemRange r2 = allocator.my_malloc();
    ASSERT_TRUE(r2.lo);
    EXPECT_EQ(r2.lo, r.lo); 
}

TEST(FixedAllocatorTest, RandomAllocFreeOrder) {
    FixedAllocator allocator;
    std::vector<MemRange> blocks;
    for (int i = 0; i < NUM_BLOCKS; ++i) {
        blocks.push_back(allocator.my_malloc());
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(blocks.begin(), blocks.end(), g);
    for (auto &b : blocks) {
        EXPECT_TRUE(allocator.my_free(b));
    }
    for (int i = 0; i < NUM_BLOCKS; ++i) {
        EXPECT_TRUE(allocator.my_malloc().lo);
    }
}

TEST(FixedAllocatorTest, OverwriteHighByte) {
    FixedAllocator allocator;
    MemRange r = allocator.my_malloc();
    ASSERT_TRUE(r.lo && r.hi);
    r.hi[0] = 0xFF; 
    EXPECT_TRUE(allocator.my_free(r));
    MemRange r2 = allocator.my_malloc();
    ASSERT_TRUE(r2.lo);
    EXPECT_EQ(r2.lo, r.lo); 
}

TEST(FixedAllocatorTest, FreeMismatchedMemRange) {
    FixedAllocator allocator;
    MemRange r = allocator.my_malloc();
    ASSERT_TRUE(r.lo);

    MemRange bad = r;
    bad.lo += 4; 
    EXPECT_TRUE(allocator.my_free(r));
    EXPECT_FALSE(allocator.my_free(bad)); 
}

TEST(FixedAllocatorTest, FreeWithMismatchedPointer) {
    FixedAllocator allocator;
    MemRange r = allocator.my_malloc();
    ASSERT_TRUE(r.lo);

    MemRange fake = r;
    fake.lo = r.lo + BLOCK_SIZE; 
    EXPECT_FALSE(allocator.my_free(fake)); 
}

TEST(FixedAllocatorTest, ConcurrentAllocationsDoNotOverlap) {
    FixedAllocator allocator;
    int num_threads = NUM_CORES;
    int allocs_per_thread = 9;

    std::atomic<bool> error_detected{false};

    auto alloc_free_task = [&](int thread_idx) {
        for (int i = 0; i < allocs_per_thread; ++i) {
            MemRange r = allocator.my_malloc();
            if (!r.lo) continue; 

            uint8_t thread_id = static_cast<uint8_t>(thread_idx);
            std::memset(r.lo, thread_id, BLOCK_SIZE);
            for (int j = 0; j < BLOCK_SIZE; ++j) {
                if (r.lo[j] != thread_id) error_detected.store(true);
            }
            allocator.my_free(r);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(alloc_free_task, i);
    }
    for (auto& t : threads) t.join();

    EXPECT_FALSE(error_detected.load());
}

TEST(FixedAllocatorTest, ConcurrentAllocationsDoNotExceedCapacity) {
    FixedAllocator allocator;
    int num_threads = NUM_CORES;
    int allocs_per_thread = 9;
    std::atomic<int> current_allocated{0};
    std::atomic<bool> overflow_detected{false};

    auto alloc_free_task = [&]() {
        for (int i = 0; i < allocs_per_thread; ++i) {
            MemRange r = allocator.my_malloc();
            if (r.lo) {
                int count = current_allocated.fetch_add(1) + 1;
                if (count > NUM_BLOCKS) overflow_detected.store(true);

                std::memset(r.lo, 0xCD, BLOCK_SIZE);

                int after_free = current_allocated.fetch_sub(1) - 1;
                EXPECT_GE(after_free, 0); 

                allocator.my_free(r);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(alloc_free_task);
    for (auto& t : threads) t.join();

    EXPECT_FALSE(overflow_detected.load());
}

