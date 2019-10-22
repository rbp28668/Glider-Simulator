#pragma once
#include "Thread.h"

class CommandInterpreter;

// Class to run the REST API request receiver in its own thread.
class APIThread :
	public Thread
{
	CommandInterpreter* pInterpreter;
public:
	APIThread(CommandInterpreter* pInterpreter);
	virtual ~APIThread();
	virtual unsigned run();
	void stop();
};

