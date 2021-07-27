#include "Cursor_Surveillance.h"

bool Cursor_Surveillance::IsCursorMoved()
{
    GetCursorPos(&CursorPos[Present_Pos]);
    if (CursorPos[Present_Pos].x != CursorPos[Past_Pos].x || CursorPos[Present_Pos].y != CursorPos[Past_Pos].y) {
        CursorPos[Past_Pos] = CursorPos[Present_Pos];
        return true;
    }
    else
    {
        return false;
    }
}
