#pragma once
#include "MessageHandler.h"
#include "IGCFlightRecorder.h"

class Prepar3D;

class IgcMessageHandler :
	public MessageHandler
{
	IGCFlightRecorder fr;
public:
	IgcMessageHandler(Prepar3D* p3d);
	virtual ~IgcMessageHandler();
	
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void listAvailableIGCFiles(const std::string& filter, std::string& output);
	void launchIgcTraffic(const std::string& file, const std::string& type,bool restart, std::string& output);
	void replayIgcFile(const std::string& file, const std::string& type, std::string& output);
	void clearIgcTraffic(std::string& output);
};

