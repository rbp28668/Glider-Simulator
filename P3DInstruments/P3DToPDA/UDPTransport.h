#pragma once
#include "transport.h"
#include "../P3DCommon/UDPClient.h"

class UDPTransport :
	public Transport
{
	UDPClient udp;
	DWORD lastSent;
	DWORD delay;


public:

	void setDelay(DWORD delay) { this->delay = delay; }

	UDPTransport(const char* host, int port);
	virtual ~UDPTransport(void);

	virtual bool canSend();
	virtual void send(std::string& str);
};

