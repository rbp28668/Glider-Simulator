#include "stdafx.h"
#include "Tug.h"
#include "APIParameters.h"
#include "TugMessageHandler.h"
#include "JSONWriter.h"
#include "Simulator.h"
#include "P3DEventCommand.h"

void TugMessageHandler::steerTug(const APIParameters& params, std::string& output)
{

	JSONWriter json(output);

	Tug* pTug = pSim->tug();

	if (pTug) {
		bool ok = false;
		double speed = params.getFloat("speed", -1);
		if (speed > 0) {
			pTug->setDesiredSpeed(speed);
			ok = true;
		}

		double heading = params.getFloat("heading", -1);
		if (heading >= 0) {
			pTug->setDesiredHeading(heading);
			ok = true;
		}

		if (ok) {
			json.add("status", "OK");
		}
		else {
			json.add("status", "FAILED")
				.add("reason", "No parameters (speed or heading) defined");
		}
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}


}

void TugMessageHandler::controlTug(const APIParameters& params, std::string& output)
{
}

void TugMessageHandler::waveOff(const APIParameters& params, std::string& output)
{
	JSONWriter json(output);

	Tug* pTug = pSim->tug();
	if (pTug) {
		pTug->waggleWings();
		json.add("status", "OK");
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}
}

void TugMessageHandler::waggle(const APIParameters& params, std::string& output)
{
	JSONWriter json(output);

	Tug* pTug = pSim->tug();
	if (pTug) {
		pTug->waggleRudder();
		json.add("status", "OK");
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}
}

bool TugMessageHandler::sendCommand(const std::string& cmd, const APIParameters& params, std::string& output) {
	DWORD dwData = params.getDWORD("value", 0);

	// Force upper case for lookup.
	std::string asUpper(cmd);
	for (std::string::iterator i = asUpper.begin(); i != asUpper.end(); ++i) {
		*i = ::toupper(*i);
	}

	bool success = false;
	JSONWriter json(output);

	Tug* pTug = pSim->tug();
	if (pTug) {
		if (pSim->getCommands()->dispatchEvent(asUpper, dwData, pTug->id())) {
			json.add("status", "FAILED")
				.add("reason", "Unable to run P3D command " + asUpper + " on tug");
		}
		else {
			success = true;
			json.add("status", "OK");
		}
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}

	return success;
}

TugMessageHandler::TugMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "tug")
	, pSim(static_cast<Simulator*>(p3d))
{
}

TugMessageHandler::~TugMessageHandler()
{
}


void TugMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "steer") {
		steerTug(params, output);
	}
	else if (cmd == "control") {
		controlTug(params, output);
	}
	else if (cmd == "wave") {
		waveOff(params, output);
	}
	else if (cmd == "waggle") {
		waggle(params, output);
	}
	else {
		sendCommand(cmd, params, output);
	}

}
