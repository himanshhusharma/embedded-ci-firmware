#include "fw/overspeed_monitor.hpp"

namespace fw {

OverspeedMonitor::OverspeedMonitor(const Config& cfg) : cfg_(cfg) {
    if (cfg_.debounce == 0U) {
        cfg_.debounce = 1U;  // 0 would mean "never switch"; treat as immediate
    }
}

std::uint16_t OverspeedMonitor::off_threshold() const {
    // Guard against hysteresis larger than the limit (unsigned underflow).
    return (cfg_.hysteresis >= cfg_.limit)
               ? 0U
               : static_cast<std::uint16_t>(cfg_.limit - cfg_.hysteresis);
}

bool OverspeedMonitor::update(std::uint16_t speed) {
    const bool wants_change =
        warning_ ? (speed <= off_threshold()) : (speed > cfg_.limit);

    if (!wants_change) {
        counter_ = 0U;
        return warning_;
    }
    if (counter_ < cfg_.debounce) {
        ++counter_;
    }
    if (counter_ >= cfg_.debounce) {
        warning_ = !warning_;
        counter_ = 0U;
    }
    return warning_;
}

void OverspeedMonitor::invalidate() { counter_ = 0U; }

}  // namespace fw
