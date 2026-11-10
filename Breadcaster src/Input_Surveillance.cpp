#include "Input_Surveillance.h"
#include <array>
#include <stdexcept>

Input_Surveillance* Input_Surveillance::active_ = nullptr;

Input_Surveillance::Input_Surveillance(PacketQueue& queue, int region_width, bool suppress_local)
    : queue_(queue), region_width_(region_width), suppress_local_(suppress_local) {
    if (active_) throw std::runtime_error("Only one hook session is supported per process.");
    active_ = this;
    keyboard_ = SetWindowsHookExW(WH_KEYBOARD_LL, keyboard_proc, GetModuleHandleW(nullptr), 0);
    mouse_ = SetWindowsHookExW(WH_MOUSE_LL, mouse_proc, GetModuleHandleW(nullptr), 0);
    if (!keyboard_ || !mouse_) {
        if (keyboard_) UnhookWindowsHookEx(keyboard_);
        if (mouse_) UnhookWindowsHookEx(mouse_);
        active_ = nullptr;
        throw std::runtime_error("Cannot install input hooks.");
    }
}

Input_Surveillance::~Input_Surveillance() {
    UnhookWindowsHookEx(keyboard_);
    UnhookWindowsHookEx(mouse_);
    active_ = nullptr;
    for (std::size_t key = 1; key < keys_.size(); ++key) {
        if (keys_[key]) queue_.push({2, static_cast<int>(key), 0});
    }
    constexpr std::array<int, 3> released{MOUSEEVENTF_LEFTUP, MOUSEEVENTF_RIGHTUP, MOUSEEVENTF_MIDDLEUP};
    for (std::size_t index = 0; index < released.size(); ++index) {
        if ((buttons_ & (1u << index)) != 0) queue_.push({0, released[index], 0});
    }
    queue_.push({5, 0, 0});
}

bool Input_Surveillance::remote_position() const noexcept {
    POINT position{};
    return GetCursorPos(&position) && position.x < 0 && position.x >= -region_width_;
}

LRESULT CALLBACK Input_Surveillance::keyboard_proc(int code, WPARAM message, LPARAM data) {
    auto* session = active_;
    if (code != HC_ACTION || !session) return CallNextHookEx(nullptr, code, message, data);
    const auto& event = *reinterpret_cast<const KBDLLHOOKSTRUCT*>(data);
    if ((event.flags & LLKHF_INJECTED) != 0 || event.vkCode < 1 || event.vkCode > 254) {
        return CallNextHookEx(nullptr, code, message, data);
    }
    const bool up = message == WM_KEYUP || message == WM_SYSKEYUP;
    const bool down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
    // Suppressed modifiers do not reach local keyboard state, so also consult
    // the keys already forwarded by this session when recognizing shutdown.
    const bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) ||
        session->keys_[VK_CONTROL] || session->keys_[VK_LCONTROL] || session->keys_[VK_RCONTROL];
    const bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) ||
        session->keys_[VK_MENU] || session->keys_[VK_LMENU] || session->keys_[VK_RMENU];
    if (event.vkCode == VK_F12 && down && control && alt) {
        PostQuitMessage(0);
        return 1;
    }
    if ((up || down) && (session->remote_position() || (up && session->keys_[event.vkCode]))) {
        const bool queued = session->queue_.push({up ? 2 : 1, static_cast<int>(event.vkCode), 0});
        if (queued) session->keys_.set(event.vkCode, !up);
        if (queued && session->suppress_local_) return 1;
    }
    return CallNextHookEx(nullptr, code, message, data);
}

LRESULT CALLBACK Input_Surveillance::mouse_proc(int code, WPARAM message, LPARAM data) {
    auto* session = active_;
    if (code != HC_ACTION || !session) return CallNextHookEx(nullptr, code, message, data);
    const auto& event = *reinterpret_cast<const MSLLHOOKSTRUCT*>(data);
    if ((event.flags & LLMHF_INJECTED) != 0) return CallNextHookEx(nullptr, code, message, data);
    int flag = 0;
    int button = -1;
    bool up = false;
    switch (message) {
    case WM_LBUTTONDOWN: flag = MOUSEEVENTF_LEFTDOWN; button = 0; break;
    case WM_LBUTTONUP: flag = MOUSEEVENTF_LEFTUP; button = 0; up = true; break;
    case WM_RBUTTONDOWN: flag = MOUSEEVENTF_RIGHTDOWN; button = 1; break;
    case WM_RBUTTONUP: flag = MOUSEEVENTF_RIGHTUP; button = 1; up = true; break;
    case WM_MBUTTONDOWN: flag = MOUSEEVENTF_MIDDLEDOWN; button = 2; break;
    case WM_MBUTTONUP: flag = MOUSEEVENTF_MIDDLEUP; button = 2; up = true; break;
    case WM_MOUSEWHEEL:
        if (session->remote_position()) {
            const int delta = static_cast<short>(HIWORD(event.mouseData));
            if (delta != 0 && session->queue_.push({4, delta, 0}) &&
                session->suppress_local_) return 1;
        }
        return CallNextHookEx(nullptr, code, message, data);
    default: return CallNextHookEx(nullptr, code, message, data);
    }
    const unsigned mask = 1u << static_cast<unsigned>(button);
    if (session->remote_position() || (up && (session->buttons_ & mask) != 0)) {
        const bool queued = session->queue_.push({0, flag, 0});
        if (queued) {
            if (up) session->buttons_ &= ~mask;
            else session->buttons_ |= mask;
        }
        if (queued && session->suppress_local_) return 1;
    }
    return CallNextHookEx(nullptr, code, message, data);
}
