#pragma once
#include "MessageHandler.h"

class RecordMessageHandler :
	public MessageHandler
{
public:
	RecordMessageHandler(Prepar3D* p3d);
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

};

