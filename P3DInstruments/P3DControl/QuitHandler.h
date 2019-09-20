#pragma once
#include "CommandInterpreter.h"
class MessageThread;

// Handler to cause shutdown of processing threads.
class QuitHandler : public MessageHandler {
private:
	std::string name;
	MessageThread* pmt;

public:
	QuitHandler(MessageThread* pmt);
	virtual const std::string& getName();
	virtual void run(const std::string& params);

};

