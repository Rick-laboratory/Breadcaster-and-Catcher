#include "UDP_Receiver.h"
#include <chrono>
struct ChronoTimer
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	ChronoTimer()
	{
		start = std::chrono::high_resolution_clock::now();
	}
	~ChronoTimer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		float ms = duration.count() * 1000.0f;
		std::cout << "Timer took " << ms << "ms" << '\n';
	}
};
int UDP_Receiver::UDP_Receiver_INIT()
{
	if (int errorcode = WSAStartup(MAKEWORD(2, 2), &WSA)){
		wprintf(L"WSAStartup function failed with error = %d\n", errorcode);
		return EXIT_FAILURE;
	}

	Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (Socket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error = %d\n", WSAGetLastError());
		return EXIT_FAILURE;
	}

	if (setsockopt(Socket, SOL_SOCKET, SO_BROADCAST, (const char *)&Broadcast, sizeof(Broadcast)) == SOCKET_ERROR){
		wprintf(L"setsockopt function failed with error = %d\n", WSAGetLastError());
		closesocket(Socket);
		return EXIT_FAILURE;
	}

	Addr_recv.sin_family = AF_INET;
	Addr_recv.sin_port = htons(PORT);
	Addr_recv.sin_addr.s_addr = INADDR_ANY;
	return 0;
}

int UDP_Receiver::UDP_Receiver_BIND()
{
	if (bind(Socket, (sockaddr*)&Addr_recv, sizeof(Addr_recv)) == SOCKET_ERROR) {
		wprintf(L"bind function failed with error = %d\n", WSAGetLastError());
		closesocket(Socket);
	}
	return 0;
}

int UDP_Receiver::UDP_Receiver_EXEC()
{
	while (true)
	{
		recv(Socket, (char*)&Buffer_recv, sizeof(Buffer_recv), 0); 
		(*MI_Obj.*func_ptr[Buffer_recv[0]])(pt);
	}
	return 0;
}