#include "stdafx.h"
#include <string>
#include <iostream>
#include <assert.h>
#include "MessageThread.h"
#include "../P3DCommon/Network.h"


class QuitHandler : public MessageHandler {
private:
	std::string name;
	MessageThread* pmt;

public:
	QuitHandler(MessageThread* pmt);
	virtual const std::string& getName();
	virtual void run(const std::string& params);

};

QuitHandler::QuitHandler(MessageThread* pmt)
 : name("quit") 
{
	assert(pmt != 0);
	this->pmt = pmt;
}

const std::string& QuitHandler::getName() {
	return name;
}

void QuitHandler::run(const std::string& params) {
	pmt->stop();
}

MessageThread::MessageThread(unsigned short port)
	: Thread(false)  // Don't start yet.
{
	this->port = port;
	quitHandler = new QuitHandler(this);
	add(quitHandler);
}


MessageThread::~MessageThread()
{
	delete quitHandler;
}

unsigned MessageThread::run()
{
	UDPSocket socket;
	socket.bind(port);
	while (!quit) {
		char buff[512];
		sockaddr_in from = socket.recvFrom(buff, sizeof(buff));

		std::string msg(buff);
		std::string cmd;
		std::string param;
		size_t pos = msg.find(':');
		if (pos == std::string::npos) {
			cmd = msg;
			param = "";
		} else {
			cmd = msg.substr(0, pos);
			param = msg.substr(pos + 1);
		}

		process(cmd, param);
	}
	return 0;
}

void MessageThread::add(MessageHandler * pHandler)
{
	handlers[pHandler->getName()] = pHandler;
}

void MessageThread::remove(MessageHandler * pHandler)
{
	handlers.erase(pHandler->getName());
}

void MessageThread::stop()
{
	this->quit = true;
}

void MessageThread::process(const std::string & cmd, const std::string & params)
{
	std::cout << "Processing " << cmd << ":" << params << std::endl;

	Handlers::iterator pos = handlers.find(cmd);
	if (pos != handlers.end()) {
		MessageHandler* handler = pos->second;
		handler->run(params);
	}
	else {
		std::cout << "No command handler for " << cmd;
	}
	
}

