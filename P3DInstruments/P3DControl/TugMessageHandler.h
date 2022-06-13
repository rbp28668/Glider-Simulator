#pragma once
#include "MessageHandler.h"
class Simulator;

class TugMessageHandler :
	public MessageHandler
{
	Simulator* pSim;

	void steerTug(const APIParameters& params, std::string& output);
	void controlTug(const APIParameters& params, std::string& output);
	void waveOff(const APIParameters& params, std::string& output);
	void waggle(const APIParameters& params, std::string& output);
	bool sendCommand(const std::string& cmd, const APIParameters& params, std::string& output);
public:

	TugMessageHandler(Prepar3D* p3d);
	~TugMessageHandler();

	void run(const std::string& cmd, const APIParameters& params, std::string& output);

};

