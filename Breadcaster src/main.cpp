#include "Broadcast_UDP.h"
#include "Cursor_Surveillance.h"
#include <atomic>
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {
std::atomic<DWORD> message_thread{0};
BOOL WINAPI stop_handler(DWORD event) {
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    const auto thread = message_thread.load();
    return thread != 0 && PostThreadMessageW(thread, WM_QUIT, 0, 0);
}

int number(std::string_view text) {
    int value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
        throw std::invalid_argument("Expected a decimal integer.");
    }
    return value;
}

void flush(PacketQueue& queue, const Broadcast_UDP& sender) {
    Command packet;
    while (queue.pop(packet)) sender.Broadcast(packet);
}

class MessageSession {
public:
    MessageSession() {
        MSG message{};
        PeekMessageW(&message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        message_thread.store(GetCurrentThreadId());
        try {
            handler_ = SetConsoleCtrlHandler(stop_handler, TRUE) != FALSE;
            if (!handler_) throw std::runtime_error("Cannot install Ctrl+C handler.");
            timer_ = SetTimer(nullptr, 0, 8, nullptr);
            if (!timer_) throw std::runtime_error("Cannot create cursor polling timer.");
        } catch (...) { cleanup(); throw; }
    }
    ~MessageSession() { cleanup(); }
    MessageSession(const MessageSession&) = delete;
    MessageSession& operator=(const MessageSession&) = delete;
    UINT_PTR timer() const { return timer_; }
private:
    void cleanup() {
        if (timer_) KillTimer(nullptr, timer_);
        if (handler_) SetConsoleCtrlHandler(stop_handler, FALSE);
        message_thread.store(0);
    }
    bool handler_ = false;
    UINT_PTR timer_ = 0;
};

void forward(const Broadcast_UDP& sender, int region_width, bool suppress) {
    PacketQueue queue;
    Cursor_Surveillance cursor;
    {
        const MessageSession session;
        const Input_Surveillance hooks(queue, region_width, suppress);
        std::cout << "Forwarding in the left region [-" << region_width << ", 0)"
                  << ". Stop with Ctrl+Alt+F12 or Ctrl+C.\n"
                  << (suppress ? "Remote-region buttons/keys are suppressed locally.\n"
                               : "Local buttons/keys remain active (mirror mode).\n");
        MSG message{};
        BOOL result = 0;
        while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
            if (message.message == WM_TIMER && message.wParam == session.timer()) cursor.Poll(queue, region_width);
            TranslateMessage(&message);
            DispatchMessageW(&message);
            flush(queue, sender);
        }
        if (result == -1) throw std::runtime_error("Message loop failed.");
        flush(queue, sender);
    } // Unhook first; queue best-effort releases and cursor restoration.
    flush(queue, sender);
    std::cout << "Stopped. Queue overflow drops: " << queue.dropped() << '\n';
}
} // namespace

int main(int argc, char* argv[]) {
    if (argc == 1 || (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h"))) {
        std::cout << "Usage: breadcaster --send IPv4 \"operation first second\" [port]\n"
                     "       breadcaster --forward IPv4 [left-width] [port] [--suppress-local]\n"
                     "       breadcaster --check\n"
                     "Defaults: port=9009; left-width=primary display width.\n"
                     "--check briefly installs/removes hooks; it sends no packets.\n";
        return 0;
    }
    try {
        const std::string_view mode = argv[1];
        if (argc == 2 && mode == "--check") {
            PacketQueue queue;
            { const Input_Surveillance hooks(queue, 0); }
            std::cout << "Keyboard and mouse hook installation/removal passed.\n";
            return 0;
        }
        bool suppress = false;
        if (mode == "--forward" && std::string_view(argv[argc - 1]) == "--suppress-local") {
            suppress = true;
            --argc;
        }
        if ((mode == "--send" && (argc < 4 || argc > 5)) ||
            (mode == "--forward" && (argc < 3 || argc > 5)) ||
            (mode != "--send" && mode != "--forward")) {
            throw std::invalid_argument("Invalid arguments. Use --help.");
        }
        const int port = argc == 5 ? number(argv[4]) : 9009;
        if (port < 1 || port > 65535) throw std::invalid_argument("Port must be 1..65535.");
        if (mode == "--send") {
            const auto packet = parse_command(argv[3]);
            const Broadcast_UDP sender(static_cast<std::uint16_t>(port), argv[2]);
            sender.Broadcast(packet);
            std::cout << "One instruction sent.\n";
        } else {
            if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) &&
                GetLastError() != ERROR_ACCESS_DENIED) {
                throw std::runtime_error("Cannot enable per-monitor DPI awareness.");
            }
            const int region_width = argc >= 4 ? number(argv[3]) : GetSystemMetrics(SM_CXSCREEN);
            if (region_width < 1 || region_width > 32768) throw std::invalid_argument("Left region width must be 1..32768.");
            const Broadcast_UDP sender(static_cast<std::uint16_t>(port), argv[2]);
            forward(sender, region_width, suppress);
        }
        return 0;
    } catch (const std::invalid_argument& error) {
        std::cerr << error.what() << '\n';
        return 2;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
