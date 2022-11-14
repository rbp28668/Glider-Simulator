#pragma once
#include "MessageHandler.h"
class Simulator;

class TugMessageHandler :
	public MessageHandler
{
	Simulator* pSim;

	void steerTug(const APIParameters& params, std::string& output);
	void waveOff(const APIParameters& params, std::string& output);
	void waggle(const APIParameters& params, std::string& output);
	bool sendCommand(const std::string& cmd, const APIParameters& params, std::string& output);
public:

	// Static entry points to support scripting
	static bool setDesiredSpeed(Simulator* pSim, double kts);
	static bool setDesiredHeading(Simulator* pSim, double heading);
	static bool waveOff(Simulator* pSim);
	static bool waggle(Simulator* pSim);
	static bool sendCommand(Simulator* pSim, const std::string& cmd, DWORD dwData);
	static bool hasReleased(Simulator* pSim);

	TugMessageHandler(Prepar3D* p3d);
	~TugMessageHandler();

	void run(const std::string& cmd, const APIParameters& params, std::string& output);

};

