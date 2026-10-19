#include "fw/speed_frame.hpp"

#include "fw/crc8.hpp"

namespace fw {
namespace {

constexpr std::uint8_t kCounterMask = 0x0FU;
constexpr std::uint8_t kValidBit = 0x01U;

std::uint8_t frame_crc(const CanPayload& f, std::uint8_t data_id) {
    std::array<std::uint8_t, 8> buf{};
    buf[0] = data_id;
    for (std::size_t i = 0U; i < 7U; ++i) {
        buf[i + 1U] = f[i];
    }
    return crc8_sae_j1850(buf.data(), buf.size());
}

}  // namespace

CanPayload encode_speed_frame(const SpeedSignal& sig, std::uint8_t counter, std::uint8_t data_id) {
    CanPayload f{};
    f[0] = static_cast<std::uint8_t>(sig.speed & 0xFFU);
    f[1] = static_cast<std::uint8_t>((sig.speed >> 8U) & 0xFFU);
    f[2] = sig.valid ? kValidBit : 0U;
    f[6] = static_cast<std::uint8_t>(counter & kCounterMask);
    f[7] = frame_crc(f, data_id);
    return f;
}

SpeedReceiver::SpeedReceiver(std::uint8_t data_id, std::uint8_t max_counter_delta)
    : data_id_(data_id), max_delta_(max_counter_delta == 0U ? 1U : max_counter_delta) {}

RxStatus SpeedReceiver::receive(const CanPayload& frame, SpeedSignal& out) {
    if (frame_crc(frame, data_id_) != frame[7]) {
        return RxStatus::CrcError;  // don't touch counter state on a corrupt frame
    }

    const std::uint8_t counter = static_cast<std::uint8_t>(frame[6] & kCounterMask);
    RxStatus status = RxStatus::Ok;
    if (!has_last_) {
        status = RxStatus::Initial;
    } else {
        const std::uint8_t delta =
            static_cast<std::uint8_t>((counter - last_counter_) & kCounterMask);
        if (delta == 0U) {
            return RxStatus::Repeated;
        }
        if (delta > max_delta_) {
            status = RxStatus::WrongSequence;
        }
    }
    last_counter_ = counter;
    has_last_ = true;

    if (status == RxStatus::WrongSequence) {
        return status;  // resynchronise on this frame, but don't trust its value
    }

    out.speed = static_cast<std::uint16_t>(frame[0] | (static_cast<std::uint16_t>(frame[1]) << 8U));
    out.valid = (frame[2] & kValidBit) != 0U;
    if (!out.valid) {
        return RxStatus::SignalInvalid;
    }
    return status;
}

}  // namespace fw
