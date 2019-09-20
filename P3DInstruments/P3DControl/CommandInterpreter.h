#pragma once
#include <map>
#include <string>

class Prepar3D;
class APIParameters;

// Class to handle a given set of messages.  The API is defined by a set of 
// message handlers, each with their own name.
class MessageHandler {
public:
	virtual const std::string& getName() = 0;
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output) = 0;
};


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

