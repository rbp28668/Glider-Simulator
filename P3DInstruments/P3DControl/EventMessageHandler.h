#pragma once
#include "MessageThread.h"
#include "P3DEventCommand.h"

class Prepar3D;

// Message handler that knows how to process basic P3D events.
class EventMessageHandler : public MessageHandler
{
	std::string name;
	Prepar3D* p3d;
	P3DEventCommand commands;
public:
	EventMessageHandler(Prepar3D* p3d);
	~EventMessageHandler();
	virtual const std::string& getName();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

};

