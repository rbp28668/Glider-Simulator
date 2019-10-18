#pragma once
#include "MessageHandler.h"
class MessageThread;

// Handler to cause shutdown of processing threads.
class QuitHandler : public MessageHandler {
private:
	MessageThread* pmt;

public:
	QuitHandler(MessageThread* pmt);
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

};

