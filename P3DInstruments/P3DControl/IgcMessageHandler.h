#pragma once
#include "MessageHandler.h"
#include "IGCFlightRecorder.h"

class Prepar3D;
class Simulator;

class IgcMessageHandler :
	public MessageHandler
{
public:
	IgcMessageHandler(Prepar3D* p3d);
	virtual ~IgcMessageHandler();
	
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	static bool listFiles(const std::string& filter, std::list<std::string>& fileList, std::string& err);
	static bool launch(Simulator* pSim, const std::string& file, const std::string& type, bool restart, std::string& err);
	static bool replay(Simulator* pSim, const std::string& file, const std::string& type, std::string& err);
	static bool clear(Simulator* pSim, std::string& err);
	static bool start(Simulator* pSim, int interval, const std::string& p1, const std::string& p2, const std::string& glider_type, const std::string& registration, const std::string& competition_class, const std::string& competition_id, std::string& err);
	static bool stop(Simulator* pSim, std::string& err);


	void listAvailableIGCFiles( const std::string& filter, std::string& output);
	void launchIgcTraffic( const std::string& file, const std::string& type,bool restart, std::string& output);
	void replayIgcFile( const std::string& file, const std::string& type, std::string& output);
	void clearIgcTraffic(std::string& output);
};
