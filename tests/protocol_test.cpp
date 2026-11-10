#include "Broadcast_UDP.h"
#include "UDP_Receiver.h"
#include "binary_protocol.hpp"
#include "packet_queue.hpp"
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
} // namespace

int main() {
    try {
        const auto negative = encode_command({3, -1, -2147483647 - 1});
        require(static_cast<unsigned char>(negative[4]) == 255 &&
                static_cast<unsigned char>(negative[7]) == 255 &&
                static_cast<unsigned char>(negative[11]) == 128, "Little-endian sign encoding failed.");
        const auto decoded = decode_command({negative.data(), negative.size()});
        require(decoded.first == -1 && decoded.second == -2147483647 - 1, "Signed decode failed.");

        UDP_Receiver receiver(0);
        const Broadcast_UDP sender(receiver.Port(), "127.0.0.1");
        for (const Command command : {Command{0, 2, 0}, Command{1, 65, 0}, Command{2, 65, 0},
                                      Command{3, -100, 200}, Command{4, -120, 0},
                                      Command{5, 0, 0}, Command{6, 0, 0}}) {
            sender.Broadcast(command);
            const auto packet = receiver.Receive();
            require(packet && packet->payload.size() == 12, "Wrong datagram size.");
            const auto actual = decode_command(packet->payload);
            require(actual.operation == command.operation && actual.first == command.first &&
                    actual.second == command.second, "Binary UDP roundtrip mismatch.");
        }
        for (const std::size_t size : {std::size_t{0}, std::size_t{4}, std::size_t{11}, std::size_t{13}}) {
            bool rejected = false;
            try { (void)decode_command(std::string(size, '\0')); }
            catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "Malformed binary packet accepted.");
        }
        auto invalid = encode_command({5, 0, 0});
        invalid[0] = 99;
        bool rejected = false;
        try { (void)decode_command({invalid.data(), invalid.size()}); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Unknown opcode accepted.");
        PacketQueue queue;
        Command command{};
        for (int pass = 0; pass < 3; ++pass) {
            for (std::size_t index = 0; index < PacketQueue::capacity; ++index) {
                require(queue.push({3, static_cast<int>(index), pass}), "Early queue overflow.");
            }
            require(!queue.push({}), "Queue overflow not detected.");
            for (std::size_t index = 0; index < PacketQueue::capacity; ++index) {
                require(queue.pop(command) && command.first == static_cast<int>(index) &&
                        command.second == pass, "Queue FIFO failed.");
            }
        }
        require(queue.dropped() == 3, "Drop count failed.");
        std::cout << "Binary compatibility, signed bounds, loopback and queue checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
