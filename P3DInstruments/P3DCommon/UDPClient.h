#pragma once

#include <winsock2.h>
#include <string>
#include "Network.h"

class UDPClient
{
	// Socket related
   
	SOCKET socketHandle;
	struct sockaddr_in si;
    int slen;
	WSASession session;
public:
	UDPClient(const char* host, int port);
	virtual ~UDPClient(void);

	void send(const std::string& msg);
};

