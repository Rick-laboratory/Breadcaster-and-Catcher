#include "Message_Interpreter.h"
//#include <iostream>

HCURSOR createBlankCursor()
{
	int cw = GetSystemMetrics(SM_CXCURSOR);
	int ch = GetSystemMetrics(SM_CYCURSOR);
	UINT8* cursorAND = new UINT8[ch * ((cw + 31) >> 2)];
	UINT8* cursorXOR = new UINT8[ch * ((cw + 31) >> 2)];
	memset(cursorAND, 0xff, ch * ((cw + 31) >> 2));
	memset(cursorXOR, 0x00, ch * ((cw + 31) >> 2));
	HCURSOR c = CreateCursor(GetModuleHandle(NULL),
	0, 0, cw, ch, cursorAND, cursorXOR);
	delete[] cursorXOR;
	delete[] cursorAND;
	return c;
}

// instr[0] = 0; MouseClick
// Instr[1] = Left/Right/press/release
void MI::MouseInput(int* x)
{
	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = *(x + 1);
	::SendInput(1, &Input, sizeof(INPUT));
}

// instr[0] = 1; Keyboard_Down
// instr[1] = KEY
void MI::Keyboard_Down(int* x)
{
	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_KEYBOARD;
	Input.ki.wVk = *(x + 1);
	::SendInput(1, &Input, sizeof(INPUT));
}

// instr[0] = 2; Keyboard_Up
// instr[1] = KEY
void MI::Keyboard_Up(int* x)
{
	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_KEYBOARD;
	Input.ki.wVk = *(x + 1);
	Input.ki.dwFlags = KEYEVENTF_KEYUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

// instr[0] = 3; CursorMove
// instr[1] = X; 
// instr[2] = Y; 
void MI::CursorMove(int* x)
{
	SetCursorPos(*(x + 1), *(x + 2));
}

// instr[0] = 4; MouseWheel
// instr[1] = Input.mi.mouseData
void MI::MouseWheel(int* x)
{
	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	Input.mi.mouseData = *(x + 1);
	Input.mi.time = 0;
	SendInput(1, &Input, sizeof(INPUT));
}

// instr[0] = 5
void MI::ShowCursor(int*)
{
	SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);
}

// instr[0] = 6
void MI::HideCursor(int*)
{
	for (int i = 0; i < 14; i++)
	{
		SetSystemCursor(createBlankCursor(), Cursors[i]);
	}
	DestroyCursor(createBlankCursor());
}
