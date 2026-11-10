#include "Broadcast_UDP.h"
#include "binary_protocol.hpp"

#include <ws2tcpip.h>
#include <stdexcept>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace {
std::runtime_error socket_error(const char* operation) {
    return std::runtime_error(std::string(operation) + " failed: " +
                              std::to_string(WSAGetLastError()));
}
} // namespace

Broadcast_UDP::Broadcast_UDP(std::uint16_t port, const std::string& destination) {
    if (port == 0) throw std::invalid_argument("The destination port must be nonzero.");
    destination_.sin_family = AF_INET;
    destination_.sin_port = htons(port);
    if (inet_pton(AF_INET, destination.c_str(), &destination_.sin_addr) != 1) {
        throw std::invalid_argument("The destination must be an IPv4 address.");
    }

    WSADATA data{};
    const int startup = WSAStartup(MAKEWORD(2, 2), &data);
    if (startup != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(startup));
    }
    try {
        socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ == INVALID_SOCKET) throw socket_error("socket");

        u_long nonblocking = 1;
        if (ioctlsocket(socket_, FIONBIO, &nonblocking) == SOCKET_ERROR) {
            throw socket_error("Set nonblocking socket");
        }
        const BOOL enabled = TRUE;
        if (setsockopt(socket_, SOL_SOCKET, SO_BROADCAST,
                       reinterpret_cast<const char*>(&enabled), sizeof(enabled)) == SOCKET_ERROR) {
            throw socket_error("setsockopt(SO_BROADCAST)");
        }
    } catch (...) {
        if (socket_ != INVALID_SOCKET) closesocket(socket_);
        WSACleanup();
        throw;
    }
}

Broadcast_UDP::~Broadcast_UDP() {
    closesocket(socket_);
    WSACleanup();
}

void Broadcast_UDP::Broadcast(const Command& command) const {
    const auto packet = encode_command(command);
    const int length = static_cast<int>(packet.size());
    const int sent = sendto(socket_, packet.data(), length, 0,
                           reinterpret_cast<const sockaddr*>(&destination_), sizeof(destination_));
    if (sent == SOCKET_ERROR) throw socket_error("sendto");
    if (sent != length) throw std::runtime_error("Incomplete datagram send.");
}
