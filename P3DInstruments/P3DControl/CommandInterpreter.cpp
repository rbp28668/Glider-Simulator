#include "stdafx.h"
#include <iostream>

#include "CommandInterpreter.h"
#include "EventMessageHandler.h"
#include "QuitHandler.h"
#include "JSONWriter.h"
#include "APIParameters.h"

CommandInterpreter::CommandInterpreter(Prepar3D* pSim)
	: pSim(pSim)
{
	add(new EventMessageHandler(pSim));

}

CommandInterpreter::~CommandInterpreter()
{
	for (Handlers::iterator iter = handlers.begin();
		iter != handlers.end();
		++iter) {
		delete iter->second;
		iter->second = 0;
	}

	handlers.clear();
}

void CommandInterpreter::add(MessageHandler* pHandler)
{
	handlers[pHandler->getName()] = pHandler;
}

void CommandInterpreter::remove(MessageHandler* pHandler)
{
	handlers.erase(pHandler->getName());
}

void CommandInterpreter::process(const std::string& cmd, const std::string& params, std::string& output)
{
	std::cout << "Processing " << cmd << ":" << params << std::endl;

	std::string handlerName;
	std::string handlerCmd;
	size_t pos = cmd.find('/');
	if (pos == std::string::npos) {
		handlerName = cmd;
		handlerCmd = "";
	}
	else {
		handlerName = cmd.substr(0, pos);
		handlerCmd = cmd.substr(pos + 1);
	}

	APIParameters apiParams(params);

	Handlers::iterator hi = handlers.find(handlerName);
	if (hi != handlers.end()) {
		MessageHandler* handler = hi->second;
		handler->run(handlerCmd, apiParams, output);
	}
	else {
		JSONWriter json(output);
		json.add("status", "FAILED")
			.add("reason", "No command handler for " + handlerName);
	}

}
