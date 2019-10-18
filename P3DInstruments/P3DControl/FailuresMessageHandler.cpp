#include "stdafx.h"
#include <algorithm>
#include "Simulator.h"
#include "FailuresMessageHandler.h"
#include "APIParameters.h"
#include "JSONWriter.h"

Failures::FAILURE_MODE FailuresMessageHandler::getMode(const APIParameters& params)
{
	Failures::FAILURE_MODE mode = Failures::FAIL;
	std::string modeStr = params.getString("mode");
	std::transform(modeStr.begin(), modeStr.end(), modeStr.begin(),
		[](unsigned char c) { return std::tolower(c); });
	if ((modeStr == "ok") || (modeStr == "false")) {
		mode = Failures::OK;
	}
	else if ((modeStr == "fail") || (modeStr == "true")) {
		mode = Failures::FAIL;
	}
	else if (modeStr == "blank") {
		mode = Failures::BLANK;
	}
	return mode;
}

FailuresMessageHandler::FailuresMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "failures")
	, pSim(static_cast<Simulator*>(p3d))
{
}

FailuresMessageHandler::~FailuresMessageHandler()
{
}

void FailuresMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "airspeed") {
		Failures::FAILURE_MODE mode = getMode(params);
		failAirspeed(mode, output);
	}
	else if (cmd == "altimeter") {
		Failures::FAILURE_MODE mode = getMode(params);
		failAltimeter(mode, output);
	}
	else if (cmd == "electrical") {
		Failures::FAILURE_MODE mode = getMode(params);
		failElectrical(mode, output);
	}
	else if (cmd == "pitot") {
		Failures::FAILURE_MODE mode = getMode(params);
		failPitot(mode, output);
	}
	else if (cmd == "turn_coordinator") {
		Failures::FAILURE_MODE mode = getMode(params);
		failTurnCoordinator(mode, output);
	}
	else if (cmd == "clear") {
		clearAll(output);
	}
	else if (cmd == "current") {
		current(output);
	}
	else {
		reportFailure("Unknown type of failure ", 0, output);
	}
}

void FailuresMessageHandler::current(std::string& output)
{
	Failures::Data data = pSim->getFailures()->current();
	JSONWriter json(output);
	json.add("status", "OK");
	json.add("airspeed", data.airspeed);
	json.add("altimeter", data.altimeter);
	json.add("electrical", data.electrical);
	json.add("pitot", data.pitot);
	json.add("turn_coordinator", data.turn_coordinator);
}

void FailuresMessageHandler::failAirspeed(Failures::FAILURE_MODE mode, std::string& output)
{
	pSim->getFailures()->failAirspeed(mode);
	reportSuccess(output);
}

void FailuresMessageHandler::failAltimeter(Failures::FAILURE_MODE mode, std::string& output)
{
	pSim->getFailures()->failAltimeter(mode);
	reportSuccess(output);
}

void FailuresMessageHandler::failElectrical(Failures::FAILURE_MODE mode, std::string& output)
{
	pSim->getFailures()->failElectrical(mode);
	reportSuccess(output);
}

void FailuresMessageHandler::failPitot(Failures::FAILURE_MODE mode, std::string& output)
{
	pSim->getFailures()->failPitot(mode);
	reportSuccess(output);
}

void FailuresMessageHandler::failTurnCoordinator(Failures::FAILURE_MODE mode, std::string& output)
{
	pSim->getFailures()->failTurnCoordinator(mode);
	reportSuccess(output);
}

void FailuresMessageHandler::clearAll(std::string& output)
{
	pSim->getFailures()->clearAll();
	reportSuccess(output);
}
