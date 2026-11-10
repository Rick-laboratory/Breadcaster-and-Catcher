#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bitset>
#include "packet_queue.hpp"

class Input_Surveillance {
public:
    Input_Surveillance(PacketQueue& queue, int region_width, bool suppress_local = false);
    ~Input_Surveillance();
    Input_Surveillance(const Input_Surveillance&) = delete;
    Input_Surveillance& operator=(const Input_Surveillance&) = delete;

private:
    static LRESULT CALLBACK keyboard_proc(int code, WPARAM message, LPARAM data);
    static LRESULT CALLBACK mouse_proc(int code, WPARAM message, LPARAM data);
    bool remote_position() const noexcept;
    static Input_Surveillance* active_;
    PacketQueue& queue_;
    int region_width_;
    bool suppress_local_;
    HHOOK keyboard_ = nullptr;
    HHOOK mouse_ = nullptr;
    std::bitset<256> keys_;
    unsigned buttons_ = 0;
};

