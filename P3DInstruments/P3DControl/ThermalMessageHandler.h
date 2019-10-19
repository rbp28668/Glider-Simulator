#pragma once
#include "MessageHandler.h"
class ThermalMessageHandler :
	public MessageHandler
{
public:
	ThermalMessageHandler(Prepar3D* p3d);
	void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void dropThermal(const APIParameters& params,std::string& output);
};

