// Host-side simulation of the speed-monitoring pipeline:
// sender -> CAN frame (CRC + alive counter) -> receiver -> filter -> overspeed warning.
// Injects a corrupted frame and a burst of lost frames to show fault handling.
#include <cstdio>

#include "fw/moving_average.hpp"
#include "fw/overspeed_monitor.hpp"
#include "fw/ring_buffer.hpp"
#include "fw/speed_frame.hpp"

namespace {

const char* to_string(fw::RxStatus s) {
    switch (s) {
        case fw::RxStatus::Ok: return "OK";
        case fw::RxStatus::Initial: return "INITIAL";
        case fw::RxStatus::CrcError: return "CRC_ERROR";
        case fw::RxStatus::Repeated: return "REPEATED";
        case fw::RxStatus::WrongSequence: return "WRONG_SEQUENCE";
        case fw::RxStatus::SignalInvalid: return "SIGNAL_INVALID";
    }
    return "?";
}

}  // namespace

int main() {
    constexpr std::uint8_t kDataId = 0x42U;
    // Speed profile in 0.1 km/h: accelerate past 120 km/h, hover at the limit, slow down.
    const std::uint16_t profile[] = {1000, 1100, 1180, 1210, 1230, 1250, 1240, 1205,
                                     1195, 1210, 1190, 1150, 1120, 1100, 1080, 1050,
                                     1020, 1000, 980,  960};

    fw::RingBuffer<fw::CanPayload, 32> bus;
    std::uint8_t counter = 0U;
    for (std::size_t i = 0U; i < sizeof(profile) / sizeof(profile[0]); ++i) {
        fw::CanPayload f = fw::encode_speed_frame({profile[i], true}, counter, kDataId);
        counter = static_cast<std::uint8_t>((counter + 1U) & 0x0FU);
        if (i == 4U) {
            f[0] ^= 0x10U;  // bit flip on the bus
        }
        if (i == 11U || i == 12U || i == 13U) {
            continue;       // frames lost
        }
        (void)bus.push(f);
    }

    fw::SpeedReceiver rx(kDataId, 2U);
    fw::MovingAverage<3> filter;
    fw::OverspeedMonitor monitor({1200U, 50U, 2U});  // 120.0 km/h, 5.0 km/h hysteresis

    std::printf("%-5s %-15s %-8s %-9s %s\n", "frame", "rx status", "raw", "filtered", "warning");
    fw::CanPayload frame{};
    int n = 0;
    while (bus.pop(frame)) {
        fw::SpeedSignal sig{};
        const fw::RxStatus st = rx.receive(frame, sig);
        if (st == fw::RxStatus::Ok || st == fw::RxStatus::Initial) {
            const std::uint16_t avg = filter.update(sig.speed);
            const bool warn = monitor.update(avg);
            std::printf("%-5d %-15s %5u.%u %6u.%u   %s\n", n, to_string(st), sig.speed / 10U,
                        sig.speed % 10U, avg / 10U, avg % 10U, warn ? "ON" : "off");
        } else {
            monitor.invalidate();
            std::printf("%-5d %-15s %-8s %-9s %s\n", n, to_string(st), "-", "-",
                        monitor.warning() ? "ON (held)" : "off (held)");
        }
        ++n;
    }
    return 0;
}
