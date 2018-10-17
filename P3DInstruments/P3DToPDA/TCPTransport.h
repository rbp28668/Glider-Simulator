#pragma once
#include "transport.h"
#include "../P3DCommon/TCPClient.h"

class TCPTransport :
	public Transport
{
	TCPClient tcp;
	DWORD lastSent;
	DWORD delay;

public:

	void setDelay(DWORD delay) { this->delay = delay; }

	TCPTransport(const char* host, int port);
	virtual ~TCPTransport(void);

	virtual bool canSend();
	virtual void send(std::string& str);

};

