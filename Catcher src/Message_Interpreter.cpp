#include "Message_Interpreter.h"
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool success, const char* operation) {
    if (!success) throw std::runtime_error(std::string(operation) + " failed.");
}
constexpr std::array<DWORD, 3> down_flags{MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_MIDDLEDOWN};
constexpr std::array<DWORD, 3> up_flags{MOUSEEVENTF_LEFTUP, MOUSEEVENTF_RIGHTUP, MOUSEEVENTF_MIDDLEUP};
} // namespace

MI::~MI() {
    for (std::size_t key = 1; key < pressed_keys_.size(); ++key) {
        if (!pressed_keys_[key]) continue;
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(key);
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(input));
    }
    for (std::size_t index = 0; index < up_flags.size(); ++index) {
        if ((pressed_buttons_ & (1u << index)) == 0) continue;
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = up_flags[index];
        SendInput(1, &input, sizeof(input));
    }
    if (hidden_) SystemParametersInfoW(SPI_SETCURSORS, 0, nullptr, 0);
}

void MI::show_cursor() {
    require(SystemParametersInfoW(SPI_SETCURSORS, 0, nullptr, 0) != FALSE, "Restore cursors");
    hidden_ = false;
}

void MI::hide_cursor() {
    if (hidden_) return;
    const int width = GetSystemMetrics(SM_CXCURSOR);
    const int height = GetSystemMetrics(SM_CYCURSOR);
    require(width > 0 && height > 0, "Read cursor size");
    const auto bytes = static_cast<std::size_t>((width + 15) / 16) * 2 * height;
    const std::vector<unsigned char> and_mask(bytes, 0xFF);
    const std::vector<unsigned char> xor_mask(bytes, 0);
    constexpr DWORD cursors[]{32512, 32513, 32514, 32515, 32516, 32642, 32643,
                              32644, 32645, 32646, 32648, 32649, 32650};
    hidden_ = true; // Restore even if a later cursor replacement fails.
    for (const DWORD id : cursors) {
        const HCURSOR cursor = CreateCursor(GetModuleHandleW(nullptr), 0, 0, width, height,
                                             and_mask.data(), xor_mask.data());
        require(cursor != nullptr, "Create blank cursor");
        // SetSystemCursor consumes the created cursor handle.
        require(SetSystemCursor(cursor, id) != FALSE, "Replace system cursor");
    }
}

void MI::Execute(const Command& command) {
    // Revalidate callers that construct Command directly, as well as network input.
    validate_command(command);
    const Command& checked = command;
    if (checked.operation == 3) {
        require(SetCursorPos(checked.first, checked.second) != FALSE, "Move cursor");
        return;
    }
    if (checked.operation == 5) { show_cursor(); return; }
    if (checked.operation == 6) { hide_cursor(); return; }

    INPUT input{};
    if (checked.operation == 1 || checked.operation == 2) {
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(checked.first);
        input.ki.dwFlags = checked.operation == 2 ? KEYEVENTF_KEYUP : 0;
    } else {
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = checked.operation == 4 ? MOUSEEVENTF_WHEEL : static_cast<DWORD>(checked.first);
        if (checked.operation == 4) input.mi.mouseData = static_cast<DWORD>(checked.first);
    }
    require(SendInput(1, &input, sizeof(input)) == 1, "SendInput");
    if (input.type == INPUT_KEYBOARD) pressed_keys_.set(checked.first, checked.operation == 1);
    if (checked.operation == 0) {
        for (std::size_t index = 0; index < down_flags.size(); ++index) {
            if (input.mi.dwFlags == down_flags[index]) pressed_buttons_ |= 1u << index;
            if (input.mi.dwFlags == up_flags[index]) pressed_buttons_ &= ~(1u << index);
        }
    }
}
