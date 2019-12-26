#pragma once

#include "MessageHandler.h"
class PanelMessageHandler :
	public MessageHandler
{

public:
	PanelMessageHandler(Prepar3D* p3d);
	virtual ~PanelMessageHandler();

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);
	void send(const std::string& host, int port, const std::string& cmd, std::string& output);
};

