#pragma once
#include "MessageHandler.h"

class LogMessageHandler :
	public MessageHandler
{
public:
	LogMessageHandler(Prepar3D* p3d);
	virtual ~LogMessageHandler();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

};

