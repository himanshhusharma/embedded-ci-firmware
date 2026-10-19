#pragma once

#include <array>
#include <cstdint>

namespace fw {

// 8-byte vehicle-speed CAN frame protected E2E-style with a CRC and an
// alive counter, so the receiver can detect corrupted, repeated and lost frames.
//
//   byte 0-1  speed, little endian, 0.1 km/h per bit
//   byte 2    bit 0 = signal valid
//   byte 3-5  reserved (0)
//   byte 6    low nibble = alive counter 0..15
//   byte 7    CRC-8 SAE J1850 over data ID + bytes 0..6
using CanPayload = std::array<std::uint8_t, 8>;

struct SpeedSignal {
    std::uint16_t speed;  // 0.1 km/h
    bool valid;
};

CanPayload encode_speed_frame(const SpeedSignal& sig, std::uint8_t counter, std::uint8_t data_id);

enum class RxStatus : std::uint8_t {
    Ok,
    Initial,         // first frame accepted, no counter history yet
    CrcError,
    Repeated,        // same counter as last frame (stuck sender)
    WrongSequence,   // too many frames lost in between
    SignalInvalid,   // frame intact, but sender flags the signal invalid
};

class SpeedReceiver {
public:
    SpeedReceiver(std::uint8_t data_id, std::uint8_t max_counter_delta);

    // Check a received frame; on Ok/Initial, `out` holds the decoded signal.
    RxStatus receive(const CanPayload& frame, SpeedSignal& out);

private:
    std::uint8_t data_id_;
    std::uint8_t max_delta_;
    std::uint8_t last_counter_ = 0U;
    bool has_last_ = false;
};

}  // namespace fw
