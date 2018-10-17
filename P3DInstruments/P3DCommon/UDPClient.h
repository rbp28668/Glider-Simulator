#pragma once

#include <winsock2.h>
#include <string>

class UDPClient
{
	// Socket related
    WSADATA wsaData;
	SOCKET socketHandle;
	struct sockaddr_in si;
    int slen;

public:
	UDPClient(const char* host, int port);
	virtual ~UDPClient(void);

	void send(const std::string& msg);
};

