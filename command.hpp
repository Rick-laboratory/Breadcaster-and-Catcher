#pragma once
#include <string_view>

struct Command {
    int operation;
    int first;
    int second;
};

/// Parse exactly three ASCII integers, optionally followed by one legacy NUL.
Command parse_command(std::string_view packet);

/// Validate a command constructed by either binary parsing or local code.
void validate_command(const Command& command);
