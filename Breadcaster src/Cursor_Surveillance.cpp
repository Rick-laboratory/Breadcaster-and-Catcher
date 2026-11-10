#include "Cursor_Surveillance.h"
#include <cstdint>
#include <limits>
#include <stdexcept>

void Cursor_Surveillance::Poll(PacketQueue& queue, int region_width) {
    POINT current{};
    if (!GetCursorPos(&current)) throw std::runtime_error("GetCursorPos failed.");
    const bool remote = current.x < 0 && current.x >= -region_width;
    if (!initialized_ || remote != remote_) {
        if (!queue.push({remote ? 5 : 6, 0, 0})) return;
    }
    if (remote && (!initialized_ || !remote_ || current.x != previous_.x || current.y != previous_.y)) {
        const auto relative_x = static_cast<std::int64_t>(current.x) + region_width;
        if (relative_x > std::numeric_limits<int>::max()) {
            throw std::runtime_error("Relative cursor X exceeds the protocol range.");
        }
        if (!queue.push({3, static_cast<int>(relative_x), static_cast<int>(current.y)})) return;
    }
    previous_ = current;
    remote_ = remote;
    initialized_ = true;
}
