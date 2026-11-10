#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bitset>
#include "command.hpp"

/// Apply validated commands. Destruction best-effort restores state owned here.
class MI {
public:
    MI() = default;
    ~MI();
    MI(const MI&) = delete;
    MI& operator=(const MI&) = delete;
    void Execute(const Command& command);

private:
    void show_cursor();
    void hide_cursor();
    bool hidden_ = false;
    std::bitset<256> pressed_keys_;
    unsigned pressed_buttons_ = 0;
};
