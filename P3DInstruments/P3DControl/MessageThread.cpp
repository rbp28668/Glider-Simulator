#include "stdafx.h"
#include <string>
#include <iostream>
#include <assert.h>
#include "MessageThread.h"
#include "../P3DCommon/Network.h"



MessageThread::MessageThread(CommandInterpreter* interpreter, unsigned short port)
	: pInterpreter(interpreter)
	, port(port)
	, quit(false)
	, Thread(false)  // Don't start yet.
{
	assert(interpreter);
	assert(port != 0);
}


MessageThread::~MessageThread()
{
}

unsigned MessageThread::run()
{
	// Takes a single URL like string and splits it into path and params as if it had
	// come from a HTTP request rather than a UDP packet.
	UDPSocket socket;
	socket.bind(port);
	while (!quit) {
		char buff[512];
		sockaddr_in from = socket.recvFrom(buff, sizeof(buff));

		std::string msg(buff);
		std::string cmd;
		std::string param;
		size_t pos = msg.find('?');
		if (pos == std::string::npos) {
			cmd = msg;
			param = "";
		} else {
			cmd = msg.substr(0, pos);
			param = msg.substr(pos + 1);
		}

		// Replace any pipe or : characters with slash to make early UDP messsages more webby
		// allows pipe separator in UDP messages
		replace(cmd, '|', '/');
		replace(cmd, ':', '/');

		process(cmd, param);
	}
	return 0;
}



void MessageThread::stop()
{
	this->quit = true;
}

// Replace any "from" chars in target with "to" chars. Returns the passed target string.
std::string& MessageThread::replace(std::string& target, char from, char to)
{
	size_t pos = target.find_first_of(from);
	while (pos != std::string::npos) {
		target[pos] = to;
		pos = target.find_first_of(from, pos);
	}
	return target;
}

void MessageThread::process(const std::string & cmd, const std::string & params)
{
	std::string output;
	pInterpreter->process(cmd, params, output);
	// UDP has no back channel so just write output to stdout.
	std::cout << output << std::endl;
}

