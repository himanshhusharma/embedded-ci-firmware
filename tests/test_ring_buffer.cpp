#include <gtest/gtest.h>

#include "fw/ring_buffer.hpp"

TEST(RingBuffer, StartsEmpty) {
    fw::RingBuffer<int, 4> rb;
    int out = 0;
    EXPECT_TRUE(rb.empty());
    EXPECT_FALSE(rb.pop(out));
    EXPECT_EQ(rb.size(), 0U);
    EXPECT_EQ(rb.capacity(), 4U);
}

TEST(RingBuffer, FifoOrder) {
    fw::RingBuffer<int, 4> rb;
    for (int i = 1; i <= 3; ++i) ASSERT_TRUE(rb.push(i));
    int out = 0;
    for (int i = 1; i <= 3; ++i) {
        ASSERT_TRUE(rb.pop(out));
        EXPECT_EQ(out, i);
    }
}

TEST(RingBuffer, RejectsPushWhenFullAndKeepsOldest) {
    fw::RingBuffer<int, 2> rb;
    EXPECT_TRUE(rb.push(1));
    EXPECT_TRUE(rb.push(2));
    EXPECT_TRUE(rb.full());
    EXPECT_FALSE(rb.push(3));
    int out = 0;
    ASSERT_TRUE(rb.pop(out));
    EXPECT_EQ(out, 1);
}

TEST(RingBuffer, WrapsAroundManyTimes) {
    fw::RingBuffer<int, 3> rb;
    int out = 0;
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(rb.push(i));
        ASSERT_TRUE(rb.pop(out));
        EXPECT_EQ(out, i);
    }
    EXPECT_TRUE(rb.empty());
}

TEST(RingBuffer, ClearResets) {
    fw::RingBuffer<int, 3> rb;
    rb.push(1);
    rb.push(2);
    rb.clear();
    EXPECT_TRUE(rb.empty());
    EXPECT_TRUE(rb.push(9));
    int out = 0;
    ASSERT_TRUE(rb.pop(out));
    EXPECT_EQ(out, 9);
}
