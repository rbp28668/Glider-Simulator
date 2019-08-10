#pragma once
#include <winsock2.h>

/* Manages a winsock session with the "resource allocation is initialisation" pattern.  Note that WSAStartup and WSACleanup 
keep internal reference counting (see https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup) and 
therefore these can be used in multiple locations without further issue*/
class WSASession
{
public:
	WSASession();
	~WSASession();
private:
	WSADATA wsaData;
};

class UDPSocket
{
	WSASession session;
public:
	UDPSocket();
	~UDPSocket();

	void sendTo(const std::string& address, unsigned short port, const char* buffer, int len, int flags = 0);
	void sendTo(sockaddr_in& address, const char* buffer, int len, int flags = 0);
	sockaddr_in recvFrom(char* buffer, int len, int flags = 0);
	void bind(unsigned short port);
private:
	SOCKET sock;
};

