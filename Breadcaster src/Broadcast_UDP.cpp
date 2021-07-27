#include "Broadcast_UDP.h"
#pragma warning(disable:4996)
#pragma warning(disable:6387)
//has to be allowed within firewall
#define PORT 9009

Broadcast_UDP::Broadcast_UDP()
{
	m_error = WSAStartup(MAKEWORD(2, 2), &m_wsa);
	if (m_error != 0) {
		std::cout << "WSAStartup failed with ErrorCode:" << m_error << "\n";
	}

	m_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &m_broadcast, sizeof(m_broadcast)) < 0) {
		std::cout << "Error setting Broadcast option\n";
		closesocket(m_socket);
	}

	m_send_addr.sin_family = AF_INET;
	m_send_addr.sin_port = htons(PORT);
	m_send_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
}

Broadcast_UDP::~Broadcast_UDP()
{
	closesocket(m_socket);
	WSACleanup();
}

void Broadcast_UDP::Broadcast(int message[])
{
	var[0] = message[0];
	var[1] = message[1];
	var[2] = message[2];
	sendto(m_socket, (char*)&var, sizeof(var), 0, (sockaddr*)&m_send_addr,sizeof(m_send_addr));
}