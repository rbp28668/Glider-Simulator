#pragma once
#include "Transport.h"

class SerialLink : public Transport
{
	HANDLE hComm;		// handle to port
	HANDLE hEvent;		// handle to event to be signalled on completion
	DWORD dwToWrite;	// bytes to write
	OVERLAPPED osWrite; // track the write

public:
	SerialLink(const char* pszPort, int bps);
	virtual ~SerialLink(void);

	virtual bool canSend();
	virtual void send(std::string& str);
};

