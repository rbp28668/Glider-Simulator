#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#include <system_error>
#include "Network.h"

#pragma comment(lib, "Ws2_32.lib")

WSASession::WSASession() {
    int ret = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        // Note, not calling WSAGetLastError: The WSAStartup function directly returns the extended error code 
        // in the return value for this function. A call to the WSAGetLastError function is not needed and should not be used.
        throw std::system_error(ret, std::system_category(), "WSAStartup Failed");
    }
}

WSASession::~WSASession() {
    ::WSACleanup();
}

UDPSocket::UDPSocket() {
	sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		throw std::system_error(::WSAGetLastError(), std::system_category(), "Error opening socket");
	}
}

UDPSocket::~UDPSocket() {
	::closesocket(sock);
}

// Sends data to a specific address/port
void UDPSocket::sendTo(const std::string& address, unsigned short port, const char* buffer, int len, int flags) {
	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = ::inet_addr(address.c_str());
	add.sin_port = ::htons(port);
	sendTo(add, buffer, len, flags);
}

// Sends to the given address/port (via sockaddr_in)
void UDPSocket::sendTo(sockaddr_in& address, const char* buffer, int len, int flags) {
	int ret = ::sendto(sock, buffer, len, flags, reinterpret_cast<SOCKADDR *>(&address), sizeof(address));
	if (ret == SOCKET_ERROR) {
		throw std::system_error(::WSAGetLastError(), std::system_category(), "sendto failed");
	}
}

// Blocking receive
sockaddr_in UDPSocket::recvFrom(char* buffer, int len, int flags)
	{
		sockaddr_in from;
		int size = sizeof(from);
		// Note use of len-1 to ensure we don't overflow buffer with null terminator.
		int ret = ::recvfrom(sock, buffer, len-1, flags, reinterpret_cast<SOCKADDR *>(&from), &size);
		if (ret == SOCKET_ERROR) {
			throw std::system_error(WSAGetLastError(), std::system_category(), "recvfrom failed");
		}

		// make the buffer zero terminated
		buffer[ret] = 0;
		return from;
	}
	
void UDPSocket::bind(unsigned short port)
	{
		sockaddr_in add;
		add.sin_family = AF_INET;
		add.sin_addr.s_addr = ::htonl(INADDR_ANY);
		add.sin_port = ::htons(port);

		int ret = ::bind(sock, reinterpret_cast<SOCKADDR *>(&add), sizeof(add));
		if (ret == SOCKET_ERROR) {
			throw std::system_error(WSAGetLastError(), std::system_category(), "Bind failed");
		}
	}

