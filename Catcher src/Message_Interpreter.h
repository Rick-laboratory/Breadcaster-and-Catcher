#pragma once
#include <windows.h>

//MI stands for Message Interpreter
class MI
{
private:
	INPUT Input = { 0 };
	const int Cursors[14] = { 
	32512,32650,32515,32649,32651,
	32513,32648,32646,32643,32645,
	32642,32644,32516,32514 };
public:
	void MouseInput(int*);
	void Keyboard_Down(int*);
	void Keyboard_Up(int*);
	void CursorMove(int*);
	void MouseWheel(int*);
	void HideCursor(int*);
	void ShowCursor(int*);
};
