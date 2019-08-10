#pragma once
#include "MessageThread.h"
#include "../P3DCommon/Prepar3D.h"
#include "P3DEventCommand.h"

class EventMessageHandler : public MessageHandler
{
	std::string name;
	Prepar3D* p3d;
	P3DEventCommand commands;
public:
	EventMessageHandler(Prepar3D* p3d);
	~EventMessageHandler();
	virtual const std::string& getName();
	virtual void run(const std::string& params);

};

