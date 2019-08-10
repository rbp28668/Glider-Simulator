#pragma once

#include<winsock2.h>
#include<string>
#include"Network.h"

class TCPClient
{
	// Socket related
	WSASession session;
	SOCKET socket;
	SOCKADDR_IN name;

	bool connect();
	int trySend(const std::string& msg);

public:

	bool isConnected() const { return socket != INVALID_SOCKET; }
	bool reconnect() { return connect(); }

	TCPClient(const char* host, int port);
	~TCPClient(void);

	void send(const std::string& msg);

};

