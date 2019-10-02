#pragma once
#include <string>
#include "MessageHandler.h"
class Simulator;
class Prepar3D;

class TrafficMessageHandler :
	public MessageHandler
{
	Simulator* pSim;

	std::string getInstallationDirectory();
	std::string getAircraftTitle(const std::string& path);
public:
	TrafficMessageHandler(Prepar3D* p3d);
	~TrafficMessageHandler();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	// Launch dumb traffic at us. Units are NM, kts, feet and degrees.
	void launch(const std::string& targetName, const std::string& targetTailNumber, double targetRange, double targetSpeed, double relativeBearing, double relativeTargetHeight);
	void availableAircraft(std::string& output);
};

