#pragma once
#include "MessageHandler.h"
#include "Simulator.h"

class ThermalMessageHandler :
	public MessageHandler
{
public:
	ThermalMessageHandler(Prepar3D* p3d);

	static bool drop(Simulator* pSim, float radius, float height, float coreRate);
	static bool drop(Simulator* pSim, float radius, float height, float coreRate, float lat, float lon);

	void run(const std::string& cmd, const APIParameters& params, std::string& output);

private:
	void dropThermal(const APIParameters& params,std::string& output);
};

