#include "command.hpp"
#include <charconv>
#include <stdexcept>

namespace {
bool whitespace(char value) { return value == ' ' || value == '\t' || value == '\r' || value == '\n'; }
} // namespace

Command parse_command(std::string_view packet) {
    if (!packet.empty() && packet.back() == '\0') packet.remove_suffix(1);
    if (packet.empty() || packet.size() > 128 || packet.find('\0') != std::string_view::npos) {
        throw std::invalid_argument("Packet must be 1..128 ASCII bytes with no embedded NUL.");
    }
    int values[3]{};
    for (int& value : values) {
        while (!packet.empty() && whitespace(packet.front())) packet.remove_prefix(1);
        const auto parsed = std::from_chars(packet.data(), packet.data() + packet.size(), value);
        if (parsed.ec != std::errc{} || parsed.ptr == packet.data() ||
            (parsed.ptr != packet.data() + packet.size() && !whitespace(*parsed.ptr))) {
            throw std::invalid_argument("Expected exactly three whitespace-separated integers.");
        }
        packet.remove_prefix(static_cast<std::size_t>(parsed.ptr - packet.data()));
    }
    while (!packet.empty() && whitespace(packet.front())) packet.remove_prefix(1);
    if (!packet.empty()) throw std::invalid_argument("Unexpected trailing data.");

    const Command command{values[0], values[1], values[2]};
    validate_command(command);
    return command;
}

void validate_command(const Command& command) {
    if (command.operation < 0 || command.operation > 6) throw std::invalid_argument("Unknown operation.");
    if (command.operation != 3 && command.second != 0) {
        throw std::invalid_argument("Unused third field must be zero.");
    }
    switch (command.operation) {
    case 0:
        if (command.first != 2 && command.first != 4 && command.first != 8 &&
            command.first != 16 && command.first != 32 && command.first != 64) {
            throw std::invalid_argument("Unsupported mouse button flag.");
        }
        break;
    case 1:
    case 2:
        if (command.first < 1 || command.first > 254) throw std::invalid_argument("Invalid virtual key.");
        break;
    case 5:
    case 6:
        if (command.first != 0) throw std::invalid_argument("Cursor visibility takes two zero fields.");
        break;
    default:
        break;
    }

}
