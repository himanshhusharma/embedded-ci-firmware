#include <gtest/gtest.h>

#include "fw/overspeed_monitor.hpp"

namespace {
fw::OverspeedMonitor make(std::uint8_t debounce = 3) {
    return fw::OverspeedMonitor({1200U, 50U, debounce});  // 120.0 km/h, 5.0 hysteresis
}
}  // namespace

TEST(OverspeedMonitor, OffBelowLimit) {
    auto m = make();
    for (int i = 0; i < 10; ++i) EXPECT_FALSE(m.update(1100));
}

TEST(OverspeedMonitor, AtLimitIsNotOverspeed) {
    auto m = make(1);
    EXPECT_FALSE(m.update(1200));
    EXPECT_TRUE(m.update(1201));
}

TEST(OverspeedMonitor, TurnsOnOnlyAfterDebounce) {
    auto m = make(3);
    EXPECT_FALSE(m.update(1300));
    EXPECT_FALSE(m.update(1300));
    EXPECT_TRUE(m.update(1300));
}

TEST(OverspeedMonitor, SpikeDoesNotTrigger) {
    auto m = make(3);
    m.update(1300);
    m.update(1300);
    m.update(1100);  // counter resets
    EXPECT_FALSE(m.update(1300));
    EXPECT_FALSE(m.update(1300));
    EXPECT_TRUE(m.update(1300));
}

TEST(OverspeedMonitor, HysteresisKeepsWarningOnNearLimit) {
    auto m = make(1);
    ASSERT_TRUE(m.update(1300));
    EXPECT_TRUE(m.update(1180));  // below limit but above 1150 off-threshold
    EXPECT_TRUE(m.update(1151));
    EXPECT_FALSE(m.update(1150));  // at off-threshold -> off
}

TEST(OverspeedMonitor, TurnsOffOnlyAfterDebounce) {
    auto m = make(2);
    m.update(1300);
    ASSERT_TRUE(m.update(1300));
    EXPECT_TRUE(m.update(1000));
    EXPECT_FALSE(m.update(1000));
}

TEST(OverspeedMonitor, InvalidateRestartsDebounce) {
    auto m = make(3);
    m.update(1300);
    m.update(1300);
    m.invalidate();
    EXPECT_FALSE(m.update(1300));
    EXPECT_FALSE(m.update(1300));
    EXPECT_TRUE(m.update(1300));
}

TEST(OverspeedMonitor, HysteresisLargerThanLimitDoesNotUnderflow) {
    fw::OverspeedMonitor m({100U, 500U, 1U});
    ASSERT_TRUE(m.update(200));
    EXPECT_TRUE(m.update(50));   // off-threshold clamps to 0, not 65136
    EXPECT_FALSE(m.update(0));
}

TEST(OverspeedMonitor, ZeroDebounceActsAsImmediate) {
    fw::OverspeedMonitor m({1200U, 50U, 0U});
    EXPECT_TRUE(m.update(1300));
}
