#pragma once
#pragma comment(lib, "Ws2_32.lib")
#include<WinSock2.h>
#include <iostream>
#include "Message_Interpreter.h"
#define PORT 9009
class UDP_Receiver
{
private:
	WSADATA WSA{ 0 };
	SOCKET Socket{ 0 };
	int Broadcast = 0;
	struct sockaddr_in Addr_recv { 0 };
	int Buffer_recv[3]{0};
	
	MI* MI_Obj;
	void(MI::* func_ptr[7])(int*) 
	{
		&MI::MouseInput,
		&MI::Keyboard_Down,
		&MI::Keyboard_Up,
		&MI::CursorMove,
		&MI::MouseWheel,
		&MI::ShowCursor,
		&MI::HideCursor
	};
	int* pt = Buffer_recv;

public:
	UDP_Receiver(MI* MI_Obj) :MI_Obj(MI_Obj) {};
	~UDP_Receiver() 
	{
		shutdown(Socket, SD_BOTH);
		closesocket(Socket);
		WSACleanup();
	};
	int UDP_Receiver_INIT();
	int UDP_Receiver_BIND();
	int UDP_Receiver_EXEC();
};