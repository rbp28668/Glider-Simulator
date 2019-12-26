#pragma once
#include "MessageThread.h"
#include "P3DEventCommand.h"
#include "MessageHandler.h"
#include "Simulator.h"

class Prepar3D;

// Message handler that knows how to process basic P3D events.
class EventMessageHandler : public MessageHandler
{
	Simulator* pSim;

public:
	EventMessageHandler(Prepar3D* p3d);
	virtual ~EventMessageHandler();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

};

