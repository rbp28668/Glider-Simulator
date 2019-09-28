#pragma once
#include <map>
#include <string>

class Prepar3D;
class APIParameters;

class MessageHandler;

// Class to define a basic P3D command interpreter.
class CommandInterpreter
{
	typedef std::map<std::string, MessageHandler*> Handlers;
	Handlers handlers;
	Prepar3D* pSim;

public:
	CommandInterpreter(Prepar3D* sim);
	~CommandInterpreter();

	void add(MessageHandler* pHandler);
	void remove(MessageHandler* pHandler);
	void process(const std::string& cmd, const std::string& params, std::string& output);

};

