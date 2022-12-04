#pragma once
#include "MessageHandler.h"
#include "Failures.h"
class Prepar3D;
class Simulator;

class FailuresMessageHandler :
	public MessageHandler
{
	Simulator* pSim;
	Failures::FAILURE_MODE getMode(const APIParameters& params);

public:
	FailuresMessageHandler(Prepar3D* p3d);
	virtual ~FailuresMessageHandler();

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void current(std::string& output);
	static void failAirspeed(Simulator* pSim, Failures::FAILURE_MODE mode, std::string& output);
	static void failAltimeter(Simulator* pSim, Failures::FAILURE_MODE mode, std::string& output);
	static void failElectrical(Simulator* pSim, Failures::FAILURE_MODE mode, std::string& output);
	static void failPitot(Simulator* pSim, Failures::FAILURE_MODE mode, std::string& output);
	static void failTurnCoordinator(Simulator* pSim, Failures::FAILURE_MODE mode, std::string& output);
	static void clearAll(Simulator* pSim, std::string& output);

};

