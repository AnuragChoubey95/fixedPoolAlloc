#include <gtest/gtest.h>
#include <set>
#include <cstring>
#include <algorithm>
#include <random>
#include "../fixAlloc.h"

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

TEST(FixedAllocatorTest, FullPoolReuse) {
    FixedAllocator allocator;
    MemRange blocks[NUM_BLOCKS];

    for (int i = 0; i < NUM_BLOCKS; ++i) {
        blocks[i] = allocator.my_malloc();
        ASSERT_TRUE(blocks[i].lo);
    }

    for (int i = 0; i < NUM_BLOCKS; ++i) {
        EXPECT_TRUE(allocator.my_free(blocks[i]));
    }

    for (int i = 0; i < NUM_BLOCKS; ++i) {
        MemRange r = allocator.my_malloc();
        EXPECT_TRUE(r.lo);
    }
}

TEST(FixedAllocatorTest, TimeBombWriteCheck) {
    FixedAllocator allocator;
    MemRange a = allocator.my_malloc();
    MemRange b = allocator.my_malloc();

    memset(a.lo, 0xDE, BLOCK_SIZE);
    memset(b.lo, 0xAD, BLOCK_SIZE);

    allocator.my_free(a);

    MemRange c = allocator.my_malloc();
    ASSERT_EQ(c.lo, a.lo); 
    EXPECT_NE(memcmp(c.lo, b.lo, BLOCK_SIZE), 0);
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
