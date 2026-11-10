#include "UDP_Receiver.h"
#include <ws2tcpip.h>
#include "Message_Interpreter.h"
#include "binary_protocol.hpp"
#include <atomic>
#include <charconv>
#include <iostream>
#include <string_view>

namespace {
std::atomic<bool> stopped{false};
BOOL WINAPI stop_handler(DWORD event) {
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    stopped.store(true);
    return TRUE;
}
} // namespace

int main(int argc, char* argv[]) {
    std::string bind_address = "127.0.0.1";
    std::string peer;
    unsigned port = 9009;
    bool apply = false;
    bool once = false;
    try {
        for (int index = 1; index < argc; ++index) {
            const std::string_view argument = argv[index];
            if (argument == "--help" || argument == "-h") {
                std::cout << "Usage: catcher [--bind IPv4] [--port 0..65535]\n"
                             "                       [--peer IPv4] [--apply] [--once]\n"
                             "Default: validate packets on 127.0.0.1:9009 without applying input.\n"
                             "--apply executes input/cursor commands. Ctrl+C stops.\n";
                return 0;
            }
            if (argument == "--apply") apply = true;
            else if (argument == "--once") once = true;
            else if ((argument == "--bind" || argument == "--peer" || argument == "--port") && index + 1 < argc) {
                const std::string_view value = argv[++index];
                if (argument == "--bind") bind_address = value;
                else if (argument == "--peer") peer = value;
                else {
                    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), port);
                    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || port > 65535) {
                        throw std::invalid_argument("Port must be 0..65535.");
                    }
                }
            } else throw std::invalid_argument("Unknown or incomplete argument. Use --help.");
        }
        if (!peer.empty()) {
            in_addr address{};
            if (inet_pton(AF_INET, peer.c_str(), &address) != 1) {
                throw std::invalid_argument("Peer must be numeric IPv4.");
            }
        }
        UDP_Receiver receiver(static_cast<std::uint16_t>(port), bind_address);
        MI interpreter;
        if (!SetConsoleCtrlHandler(stop_handler, TRUE)) throw std::runtime_error("Cannot install Ctrl+C handler.");
        struct Cleanup { ~Cleanup() { SetConsoleCtrlHandler(stop_handler, FALSE); } } cleanup;
        std::cout << "Listening on " << bind_address << ':' << receiver.Port()
                  << (apply ? " (apply)" : " (dry run)") << std::endl;
        while (!stopped.load()) {
            const auto packet = receiver.Receive();
            if (!packet || (!peer.empty() && packet->sender != peer)) continue;
            try {
                const auto command = decode_command(packet->payload);
                if (apply) interpreter.Execute(command);
                std::cout << (apply ? "Applied" : "Validated") << " operation " << command.operation
                          << " from " << packet->sender << std::endl;
            } catch (const std::invalid_argument& error) {
                std::cerr << "Rejected packet: " << error.what() << '\n';
                if (once) return 2;
                continue;
            }
            if (once) break;
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
