#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace fw {

// Integer moving-average filter over the last Window samples.
// Keeps a running sum so each update is O(1). Until the window fills,
// the average is taken over the samples received so far.
template <std::size_t Window>
class MovingAverage {
    static_assert(Window > 0U, "Window must be non-zero");

public:
    std::uint16_t update(std::uint16_t sample) {
        if (count_ == Window) {
            sum_ -= samples_[index_];
        } else {
            ++count_;
        }
        samples_[index_] = sample;
        sum_ += sample;
        index_ = (index_ + 1U) % Window;
        return value();
    }

    // Rounded to nearest integer.
    std::uint16_t value() const {
        if (count_ == 0U) {
            return 0U;
        }
        return static_cast<std::uint16_t>((sum_ + (count_ / 2U)) / count_);
    }

    void reset() {
        samples_.fill(0U);
        sum_ = 0U;
        index_ = 0U;
        count_ = 0U;
    }

private:
    std::array<std::uint16_t, Window> samples_{};
    std::uint32_t sum_ = 0U;  // Window * 0xFFFF fits for Window <= 65537
    std::size_t index_ = 0U;
    std::size_t count_ = 0U;
};

}  // namespace fw
