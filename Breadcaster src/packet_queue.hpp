#pragma once
#include "command.hpp"
#include <array>
#include <cstddef>

/// Fixed storage used only on the hook/message-loop thread.
/// Callbacks enqueue; the message loop drains outside callback execution.
class PacketQueue {
public:
    static constexpr std::size_t capacity = 512;
    bool push(const Command& packet) noexcept {
        if (size_ == capacity) { ++dropped_; return false; }
        packets_[(front_ + size_) % capacity] = packet;
        ++size_;
        return true;
    }
    bool pop(Command& packet) noexcept {
        if (size_ == 0) return false;
        packet = packets_[front_];
        front_ = (front_ + 1) % capacity;
        --size_;
        return true;
    }
    std::size_t dropped() const noexcept { return dropped_; }
private:
    std::array<Command, capacity> packets_{};
    std::size_t front_ = 0;
    std::size_t size_ = 0;
    std::size_t dropped_ = 0;
};
