#include "stdafx.h"
#include "LogMessageHandler.h"
#include "Simulator.h"
#include "Logger.h"
#include "APIParameters.h"

LogMessageHandler::LogMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "log")
{

}

LogMessageHandler::~LogMessageHandler()
{
}

void LogMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "logon") {
		Simulator* sim = static_cast<Simulator*>(p3d);
		std::string number = params.getString("number");
		std::string name = params.getString("name"); 
		sim->getLogger()->logSession(sim,"logon " + number + "," + name);
		reportSuccess(output);
	} else if (cmd == "logoff") {
		Simulator* sim = static_cast<Simulator*>(p3d);
		sim->getLogger()->logSession(sim,"logoff");
		reportSuccess(output);
	} else if (cmd == "info") {
		Simulator* sim = static_cast<Simulator*>(p3d);
		std::string message = params.getString("message");
		sim->getLogger()->info(sim,message);
		reportSuccess(output);
	} else if (cmd == "warn") {
		Simulator* sim = static_cast<Simulator*>(p3d);
		std::string message = params.getString("message");
		sim->getLogger()->warn(sim,message);
		reportSuccess(output);
	}
	else if (cmd == "error") {
		Simulator* sim = static_cast<Simulator*>(p3d);
		std::string message = params.getString("message");
		sim->getLogger()->error(sim,message);
		reportSuccess(output);
	}
    else {
		reportFailure("Unkown log command ", 0, output);
	}
}
