#pragma once

#include <array>
#include <cstddef>

namespace fw {

// Fixed-capacity FIFO with static storage: no heap, no exceptions.
// push() refuses new items when full, so the oldest data is never silently lost.
template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity > 0U, "Capacity must be non-zero");

public:
    bool push(const T& item) {
        if (full()) {
            return false;
        }
        buf_[head_] = item;
        head_ = next(head_);
        ++count_;
        return true;
    }

    bool pop(T& out) {
        if (empty()) {
            return false;
        }
        out = buf_[tail_];
        tail_ = next(tail_);
        --count_;
        return true;
    }

    void clear() {
        head_ = 0U;
        tail_ = 0U;
        count_ = 0U;
    }

    bool empty() const { return count_ == 0U; }
    bool full() const { return count_ == Capacity; }
    std::size_t size() const { return count_; }
    static constexpr std::size_t capacity() { return Capacity; }

private:
    static constexpr std::size_t next(std::size_t i) { return (i + 1U) % Capacity; }

    std::array<T, Capacity> buf_{};
    std::size_t head_ = 0U;
    std::size_t tail_ = 0U;
    std::size_t count_ = 0U;
};

}  // namespace fw
