#pragma once
#include "Input_Surveillance.h"

class Cursor_Surveillance {
public:
    /// Sample the cursor and enqueue visibility/motion only when their state changes.
    void Poll(PacketQueue& queue, int region_width);
private:
    POINT previous_{};
    bool initialized_ = false;
    bool remote_ = false;
};

