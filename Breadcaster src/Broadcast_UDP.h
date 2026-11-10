#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <cstdint>
#include <string>
#include "command.hpp"

class Broadcast_UDP {
public:
    explicit Broadcast_UDP(std::uint16_t port = 9009,
                           const std::string& destination = "127.0.0.1");
    ~Broadcast_UDP();
    Broadcast_UDP(const Broadcast_UDP&) = delete;
    Broadcast_UDP& operator=(const Broadcast_UDP&) = delete;
    void Broadcast(const Command& command) const;
private:
    SOCKET socket_ = INVALID_SOCKET;
    sockaddr_in destination_{};
};
