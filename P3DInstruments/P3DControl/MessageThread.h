#pragma once
#include "Thread.h"
#include <map>

class MessageHandler {
public:
	virtual const std::string& getName() = 0;
	virtual void run(const std::string& params) = 0;
};

class MessageThread : public Thread
{
public:
	MessageThread(unsigned short port);
	~MessageThread();
	virtual unsigned run();
	void add(MessageHandler* pHandler);
	void remove(MessageHandler* pHandler);
	void stop();

private:
	unsigned short port;
	bool quit;
	typedef std::map<std::string, MessageHandler*> Handlers;
	Handlers handlers;
	MessageHandler* quitHandler;
	void process(const std::string& cmd, const std::string& params);

};

