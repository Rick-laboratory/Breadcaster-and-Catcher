#pragma once
#include "command.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>

using WirePacket = std::array<char, 12>;

inline WirePacket encode_command(const Command& command) {
    validate_command(command);
    const std::array<int, 3> values{command.operation, command.first, command.second};
    WirePacket packet{};
    for (std::size_t field = 0; field < values.size(); ++field) {
        const auto value = static_cast<std::uint32_t>(values[field]);
        for (unsigned byte = 0; byte < 4; ++byte) {
            packet[field * 4 + byte] = static_cast<char>((value >> (byte * 8)) & 0xFFu);
        }
    }
    return packet;
}

inline Command decode_command(std::string_view packet) {
    if (packet.size() != 12) throw std::invalid_argument("Expected exactly twelve protocol bytes.");
    std::array<int, 3> values{};
    static_assert(std::numeric_limits<int>::max() == 2147483647, "The API requires 32-bit int.");
    for (std::size_t field = 0; field < values.size(); ++field) {
        std::uint32_t value = 0;
        for (unsigned byte = 0; byte < 4; ++byte) {
            value |= static_cast<std::uint32_t>(static_cast<unsigned char>(packet[field * 4 + byte]))
                     << (byte * 8);
        }
        // Define the signed conversion explicitly instead of relying on casts
        // from out-of-range unsigned values.
        values[field] = value <= 2147483647u ? static_cast<int>(value)
            : -1 - static_cast<int>(std::numeric_limits<std::uint32_t>::max() - value);
    }
    const Command command{values[0], values[1], values[2]};
    validate_command(command);
    return command;
}
