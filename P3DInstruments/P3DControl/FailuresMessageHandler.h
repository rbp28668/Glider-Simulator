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
	~FailuresMessageHandler();

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void current(std::string& output);
	void failAirspeed(Failures::FAILURE_MODE mode, std::string& output);
	void failAltimeter(Failures::FAILURE_MODE mode, std::string& output);
	void failElectrical(Failures::FAILURE_MODE mode, std::string& output);
	void failPitot(Failures::FAILURE_MODE mode, std::string& output);
	void failTurnCoordinator(Failures::FAILURE_MODE mode, std::string& output);
	void clearAll(std::string& output);

};

