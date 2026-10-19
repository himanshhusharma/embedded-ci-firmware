#include <gtest/gtest.h>

#include "fw/moving_average.hpp"

TEST(MovingAverage, EmptyIsZero) {
    fw::MovingAverage<4> f;
    EXPECT_EQ(f.value(), 0U);
}

TEST(MovingAverage, AveragesPartialWindow) {
    fw::MovingAverage<4> f;
    EXPECT_EQ(f.update(10), 10U);
    EXPECT_EQ(f.update(20), 15U);
}

TEST(MovingAverage, SlidesOverFullWindow) {
    fw::MovingAverage<3> f;
    f.update(3);
    f.update(6);
    f.update(9);           // (3+6+9)/3 = 6
    EXPECT_EQ(f.value(), 6U);
    EXPECT_EQ(f.update(12), 9U);  // (6+9+12)/3 = 9
}

TEST(MovingAverage, RoundsToNearest) {
    fw::MovingAverage<2> f;
    f.update(1);
    EXPECT_EQ(f.update(2), 2U);  // 1.5 -> 2
}

TEST(MovingAverage, NoOverflowAtMaxInput) {
    fw::MovingAverage<16> f;
    for (int i = 0; i < 40; ++i) f.update(0xFFFF);
    EXPECT_EQ(f.value(), 0xFFFF);
}

TEST(MovingAverage, ResetClearsHistory) {
    fw::MovingAverage<3> f;
    f.update(100);
    f.reset();
    EXPECT_EQ(f.update(4), 4U);
}
