#pragma once
#include<stdio.h>
#include <Windows.h>
#include<iostream>
#include<string>

class Input_Surveillance
{
public:
	Input_Surveillance();
	~Input_Surveillance();
	bool IsInputDone();
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
private:
	HHOOK hhkLowLevelKybd;
	HHOOK hhLowLevelMouse;
};

