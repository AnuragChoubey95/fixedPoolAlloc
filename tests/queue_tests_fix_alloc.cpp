#include <gtest/gtest.h>
#include <cstring>
#include "../msgQueueFixAlloc.h"

TEST(MessageQueueFixAllocTest, EnqueueIncreasesSize) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0xAA};
    size_t prev_size = q.size();
    ASSERT_TRUE(q.enqueue(msg));
    EXPECT_EQ(q.size(), prev_size + 1);
}

TEST(MessageQueueFixAllocTest, DequeueDecreasesSize) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0xAA};
    uint8_t out[BLOCK_SIZE];
    q.enqueue(msg);
    size_t prev_size = q.size();
    ASSERT_TRUE(q.dequeue(out));
    EXPECT_EQ(q.size(), prev_size - 1);
}

TEST(MessageQueueFixAllocTest, FIFOBehavior) {
    MessageQueueFixAlloc q;
    uint8_t msg1[BLOCK_SIZE] = {0x01};
    uint8_t msg2[BLOCK_SIZE] = {0x02};
    uint8_t out[BLOCK_SIZE];

    ASSERT_TRUE(q.enqueue(msg1));
    ASSERT_TRUE(q.enqueue(msg2));
    ASSERT_TRUE(q.dequeue(out));
    EXPECT_EQ(memcmp(out, msg1, BLOCK_SIZE), 0);
    ASSERT_TRUE(q.dequeue(out));
    EXPECT_EQ(memcmp(out, msg2, BLOCK_SIZE), 0);
}

TEST(MessageQueueFixAllocTest, FullQueueEnqueueFails) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0xFF};
    for (int i = 0; i < QUEUE_MAX_SIZE; ++i) {
        ASSERT_TRUE(q.enqueue(msg));
    }
    EXPECT_FALSE(q.enqueue(msg));
}

TEST(MessageQueueFixAllocTest, EmptyQueueDequeueFails) {
    MessageQueueFixAlloc q;
    uint8_t out[BLOCK_SIZE];
    EXPECT_FALSE(q.dequeue(out));
}

TEST(MessageQueueFixAllocTest, NoMemoryLeaksUnderFullCycle) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0x11};
    uint8_t out[BLOCK_SIZE];
    for (int i = 0; i < QUEUE_MAX_SIZE; ++i) q.enqueue(msg);
    for (int i = 0; i < QUEUE_MAX_SIZE; ++i) ASSERT_TRUE(q.dequeue(out));
    EXPECT_EQ(q.size(), 0);
    ASSERT_TRUE(q.enqueue(msg)); 

TEST(MessageQueueFixAllocTest, AllocatorNeverOvercommits) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0xAB};
    for (int i = 0; i < NUM_BLOCKS; ++i) ASSERT_TRUE(q.enqueue(msg));
    EXPECT_FALSE(q.enqueue(msg));
}

TEST(MessageQueueFixAllocTest, WraparoundBehavior) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0x22};
    uint8_t out[BLOCK_SIZE];
    for (int i = 0; i < NUM_BLOCKS; ++i) q.enqueue(msg);
    for (int i = 0; i < NUM_BLOCKS; ++i) ASSERT_TRUE(q.dequeue(out));
    for (int i = 0; i < NUM_BLOCKS; ++i) ASSERT_TRUE(q.enqueue(msg));
    EXPECT_EQ(q.size(), NUM_BLOCKS);
}

TEST(MessageQueueFixAllocTest, PayloadIntegrity) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE];
    uint8_t out[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; ++i) msg[i] = i;
    ASSERT_TRUE(q.enqueue(msg));
    ASSERT_TRUE(q.dequeue(out));
    EXPECT_EQ(memcmp(msg, out, BLOCK_SIZE), 0);
}

TEST(MessageQueueFixAllocTest, StressReuseLoop) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0xAA};
    uint8_t out[BLOCK_SIZE];
    for (int cycle = 0; cycle < 1000; ++cycle) {
        ASSERT_TRUE(q.enqueue(msg));
        ASSERT_TRUE(q.dequeue(out));
    }
    EXPECT_EQ(q.size(), 0);
}

TEST(MessageQueueFixAllocTest, ExhaustionRecovery) {
    MessageQueueFixAlloc q;
    uint8_t msg[BLOCK_SIZE] = {0xAA};
    uint8_t out[BLOCK_SIZE];
    for (int i = 0; i < NUM_BLOCKS; ++i) ASSERT_TRUE(q.enqueue(msg));
    EXPECT_FALSE(q.enqueue(msg));
    ASSERT_TRUE(q.dequeue(out));
    ASSERT_TRUE(q.enqueue(msg)); 
}
