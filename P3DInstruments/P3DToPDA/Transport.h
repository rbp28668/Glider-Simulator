#pragma once
#include <string>
class Transport
{
public:
	virtual bool canSend() = 0;
	virtual void send(std::string& str) = 0;

	Transport(void);
	virtual ~Transport(void);
};

