#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <array>
#include <cstdint>
#include <optional>
#include <string>

struct Datagram {
    std::string payload;
    std::string sender;
};

class UDP_Receiver {
public:
    explicit UDP_Receiver(std::uint16_t port = 9009, const std::string& address = "127.0.0.1");
    ~UDP_Receiver();
    UDP_Receiver(const UDP_Receiver&) = delete;
    UDP_Receiver& operator=(const UDP_Receiver&) = delete;
    /// Nullopt on a 200 ms idle timeout. Transport failures throw.
    std::optional<Datagram> Receive();
    std::uint16_t Port() const noexcept { return port_; }

private:
    SOCKET socket_ = INVALID_SOCKET;
    std::uint16_t port_ = 0;
    std::array<char, 65507> buffer_{};
};
