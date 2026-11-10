#include "UDP_Receiver.h"
#include <ws2tcpip.h>
#include <stdexcept>

namespace {
std::runtime_error socket_error(const char* operation) {
    return std::runtime_error(std::string(operation) + " failed: " + std::to_string(WSAGetLastError()));
}
} // namespace

UDP_Receiver::UDP_Receiver(std::uint16_t port, const std::string& address) {
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    if (inet_pton(AF_INET, address.c_str(), &local.sin_addr) != 1) {
        throw std::invalid_argument("Bind address must be numeric IPv4.");
    }
    WSADATA data{};
    const int startup = WSAStartup(MAKEWORD(2, 2), &data);
    if (startup != 0) throw std::runtime_error("WSAStartup failed: " + std::to_string(startup));
    try {
        socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ == INVALID_SOCKET) throw socket_error("socket");
        const DWORD timeout = 200;
        if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == SOCKET_ERROR) {
            throw socket_error("Set receive timeout");
        }
        if (bind(socket_, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR) {
            throw socket_error("bind");
        }
        int length = sizeof(local);
        if (getsockname(socket_, reinterpret_cast<sockaddr*>(&local), &length) == SOCKET_ERROR) {
            throw socket_error("getsockname");
        }
        port_ = ntohs(local.sin_port);
    } catch (...) {
        if (socket_ != INVALID_SOCKET) closesocket(socket_);
        WSACleanup();
        throw;
    }
}

UDP_Receiver::~UDP_Receiver() {
    closesocket(socket_);
    WSACleanup();
}

std::optional<Datagram> UDP_Receiver::Receive() {
    sockaddr_in sender{};
    int length = sizeof(sender);
    const int received = recvfrom(socket_, buffer_.data(), static_cast<int>(buffer_.size()), 0,
                                  reinterpret_cast<sockaddr*>(&sender), &length);
    if (received == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) return std::nullopt;
        throw socket_error("recvfrom");
    }
    char address[INET_ADDRSTRLEN]{};
    if (!inet_ntop(AF_INET, &sender.sin_addr, address, sizeof(address))) throw socket_error("inet_ntop");
    return Datagram{{buffer_.data(), static_cast<std::size_t>(received)}, address};
}
