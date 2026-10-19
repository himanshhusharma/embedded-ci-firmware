#pragma once

#include <cstdint>

namespace fw {

// Raises an overspeed warning with hysteresis and debouncing, so a speed
// signal hovering around the limit doesn't make the warning flicker.
//
//   ON  after `debounce` consecutive samples with speed >  limit
//   OFF after `debounce` consecutive samples with speed <= limit - hysteresis
//
// Speeds are in 0.1 km/h units (e.g. 1200 = 120.0 km/h).
class OverspeedMonitor {
public:
    struct Config {
        std::uint16_t limit;
        std::uint16_t hysteresis;
        std::uint8_t debounce;
    };

    explicit OverspeedMonitor(const Config& cfg);

    // Feed one sample; returns the warning state after this sample.
    bool update(std::uint16_t speed);

    // Treat the input as invalid (e.g. CRC failure): hold the last state and
    // restart debouncing so stale counts don't trigger a change.
    void invalidate();

    bool warning() const { return warning_; }

private:
    std::uint16_t off_threshold() const;

    Config cfg_;
    bool warning_ = false;
    std::uint8_t counter_ = 0U;
};

}  // namespace fw
