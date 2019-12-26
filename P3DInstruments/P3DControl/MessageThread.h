
#pragma once
#include "Thread.h"
#include "CommandInterpreter.h"

// Thread to receive UDP messages.
class MessageThread : public Thread
{
public:
	MessageThread(CommandInterpreter* pInterpreter, unsigned short port);
	~MessageThread();
	virtual unsigned run();
	void stop();
	std::string& replace(std::string& target, char from, char to);

private:
	unsigned short port;
	bool quit;
	CommandInterpreter* pInterpreter;

	void process(const std::string& cmd, const std::string& params);

};

