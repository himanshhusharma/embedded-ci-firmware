#include <gtest/gtest.h>

#include "fw/speed_frame.hpp"

namespace {
constexpr std::uint8_t kId = 0x42;
}

TEST(SpeedFrame, EncodesLayout) {
    const auto f = fw::encode_speed_frame({0x1234, true}, 5, kId);
    EXPECT_EQ(f[0], 0x34);
    EXPECT_EQ(f[1], 0x12);
    EXPECT_EQ(f[2], 0x01);
    EXPECT_EQ(f[3] | f[4] | f[5], 0);
    EXPECT_EQ(f[6], 5);
}

TEST(SpeedFrame, RoundTrip) {
    fw::SpeedReceiver rx(kId, 2);
    fw::SpeedSignal out{};
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1234, true}, 0, kId), out), fw::RxStatus::Initial);
    EXPECT_EQ(out.speed, 1234);
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1240, true}, 1, kId), out), fw::RxStatus::Ok);
    EXPECT_EQ(out.speed, 1240);
}

TEST(SpeedFrame, DetectsCorruption) {
    fw::SpeedReceiver rx(kId, 2);
    fw::SpeedSignal out{};
    auto f = fw::encode_speed_frame({1000, true}, 0, kId);
    f[0] ^= 0x01;
    EXPECT_EQ(rx.receive(f, out), fw::RxStatus::CrcError);
}

TEST(SpeedFrame, DataIdIsPartOfCrc) {
    // A frame from another sender (different data ID) must not be accepted.
    fw::SpeedReceiver rx(kId, 2);
    fw::SpeedSignal out{};
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1000, true}, 0, 0x43), out), fw::RxStatus::CrcError);
}

TEST(SpeedFrame, DetectsRepeatedFrame) {
    fw::SpeedReceiver rx(kId, 2);
    fw::SpeedSignal out{};
    const auto f = fw::encode_speed_frame({1000, true}, 3, kId);
    rx.receive(f, out);
    EXPECT_EQ(rx.receive(f, out), fw::RxStatus::Repeated);
}

TEST(SpeedFrame, ToleratesAllowedLossAndFlagsLargerGap) {
    fw::SpeedReceiver rx(kId, 2);
    fw::SpeedSignal out{};
    rx.receive(fw::encode_speed_frame({1000, true}, 0, kId), out);
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1000, true}, 2, kId), out), fw::RxStatus::Ok);  // 1 lost
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1000, true}, 6, kId), out), fw::RxStatus::WrongSequence);
    // resynchronised: next consecutive frame is fine again
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1000, true}, 7, kId), out), fw::RxStatus::Ok);
}

TEST(SpeedFrame, CounterWrapsAt16) {
    fw::SpeedReceiver rx(kId, 1);
    fw::SpeedSignal out{};
    rx.receive(fw::encode_speed_frame({1000, true}, 15, kId), out);
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1000, true}, 0, kId), out), fw::RxStatus::Ok);
}

TEST(SpeedFrame, CorruptFrameDoesNotAdvanceCounter) {
    fw::SpeedReceiver rx(kId, 1);
    fw::SpeedSignal out{};
    rx.receive(fw::encode_speed_frame({1000, true}, 0, kId), out);
    auto bad = fw::encode_speed_frame({1000, true}, 1, kId);
    bad[7] ^= 0xFF;
    ASSERT_EQ(rx.receive(bad, out), fw::RxStatus::CrcError);
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({1000, true}, 1, kId), out), fw::RxStatus::Ok);
}

TEST(SpeedFrame, InvalidSignalFlag) {
    fw::SpeedReceiver rx(kId, 2);
    fw::SpeedSignal out{};
    EXPECT_EQ(rx.receive(fw::encode_speed_frame({0, false}, 0, kId), out), fw::RxStatus::SignalInvalid);
    EXPECT_FALSE(out.valid);
}
