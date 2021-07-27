#pragma once
#include<Windows.h>
#define Present_Pos 0
#define Past_Pos 1
#define Temp_pos 2
class Cursor_Surveillance
{
public:
	POINT& GetPresentPos() { return CursorPos[Present_Pos]; }
	bool IsCursorMoved();
private:
	POINT CursorPos[3];
};

