#include "stdafx.h"
#include "Tug.h"
#include "APIParameters.h"
#include "TugMessageHandler.h"
#include "JSONWriter.h"
#include "Simulator.h"
#include "P3DEventCommand.h"



/// <summary>
/// Manages the /tug/steer message
/// </summary>
/// <param name="params">should contain speed or heading parameters</param>
/// <param name="output">is the string to write any JSON output to.</param>
void TugMessageHandler::steerTug(const APIParameters& params, std::string& output)
{

	JSONWriter json(output);

	Tug* pTug = pSim->tug();

	if (pTug) {
		bool ok = false;
		double speed = params.getFloat("speed", -1);
		if (speed > 0) {
			setDesiredSpeed(pSim, speed);
			ok = true;
		}

		double heading = params.getFloat("heading", -1);
		if (heading >= 0) {
			setDesiredHeading(pSim, heading);
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

/// <summary>
/// Manages the /tug/steer message
/// </summary>
/// <param name="params">should contain speed or heading parameters</param>
/// <param name="output">is the string to write any JSON output to.</param>
void TugMessageHandler::turn(const APIParameters& params, std::string& output)
{

	JSONWriter json(output);

	Tug* pTug = pSim->tug();

	if (pTug) {
		double degrees = params.getFloat("degrees", 0);
		turnTug(pSim, degrees);
		json.add("status", "OK");
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}
}


void TugMessageHandler::waveOff(const APIParameters& params, std::string& output)
{
	JSONWriter json(output);

	if (waveOff(pSim)) {
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

	if (waggle(pSim)) {
		json.add("status", "OK");
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}
}

void TugMessageHandler::reportPosition(const APIParameters& params, std::string& output)
{
	JSONWriter json(output);

	Tug* pTug = pSim->tug();

	if (pTug) {
			json.add("status", "OK");
			json.object("position");

			json.add("latitude", pTug->getLatitude());
			json.add("longitude", pTug->getLongitude());
			json.add("altitude", pTug->getAltitude());
			json.add("pitch", pTug->getPitch());
			json.add("bank", pTug->getBank());
			json.add("heading", pTug->getHeading());
			json.add("on_tow", !pTug->hasReleased());

			json.end(); // of position
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}

}

bool TugMessageHandler::sendCommand(const std::string& cmd, const APIParameters& params, std::string& output) {
	DWORD dwData = params.getDWORD("value", 0);

	bool success = false;
	JSONWriter json(output);

	Tug* pTug = pSim->tug();
	if (pTug) {
		if (sendCommand(pSim, cmd, dwData)) {
			success = true;
			json.add("status", "OK");
		} else {
			json.add("status", "FAILED")
				.add("reason", "Unable to run P3D command " + cmd + " on tug");
		}
	}
	else {
		json.add("status", "FAILED")
			.add("reason", "No tug defined ");
	}

	return success;
}

/// <summary>
/// Sets the desired speed of the tug.
/// </summary>
/// <param name="pSim">Points to the main Simulator object</param>
/// <param name="kts">is the desired speed in kts</param>
/// <returns>true if there is a tug defined</returns>
bool TugMessageHandler::setDesiredSpeed(Simulator* pSim, double kts)
{
	Tug* pTug = pSim->tug();

	if (pTug) {
		if (kts > 0) {
			pTug->setDesiredSpeed(kts);
		}
	}
	return pTug != 0;
}

/// <summary>
/// Sets the desired heading of the tug (if any)
/// </summary>
/// <param name="pSim">Points to the simulator</param>
/// <param name="heading">Desired heading (in degrees)</param>
/// <returns>true if there is a tug defined</returns>
bool TugMessageHandler::setDesiredHeading(Simulator* pSim, double heading)
{
	Tug* pTug = pSim->tug();

	if (pTug) {
		if (heading >= 0) {
			pTug->setDesiredHeading(heading);
		}
	}
	return pTug != 0;
}

bool TugMessageHandler::turnTug(Simulator* pSim, double degrees) {
	Tug* pTug = pSim->tug();

	if (pTug) {
		pTug->turn(degrees);
	}
	return pTug != 0;

}

/// <summary>
/// Gets the tug (if any) to send a wave-off signal.
/// </summary>
/// <param name="pSim">Points to the simulator</param>
/// <returns>true if there is a tug defined</returns>
bool TugMessageHandler::waveOff(Simulator* pSim)
{
	Tug* pTug = pSim->tug();
	if (pTug) {
		pTug->waggleWings();
	}
	return pTug != 0;
}

/// <summary>
/// Gets the tug (if any) to send a "problem" i.e. brakes out signal.
/// </summary>
/// <param name="pSim">Points to the simulator</param>
/// <returns>true if there is a tug defined</returns>
bool TugMessageHandler::waggle(Simulator* pSim)
{
	Tug* pTug = pSim->tug();
	if (pTug) {
		pTug->waggleRudder();
	}
	return pTug != 0;
}

/// <summary>
/// Sends a command to the tug (if any)
/// </summary>
/// <param name="pSim">Points to the simulator</param>
/// <param name="cmd">is the command to send</param>
/// <param name="dwData">contains any parameter to the command</param>
/// <returns>true for success</returns>
bool TugMessageHandler::sendCommand(Simulator* pSim, const std::string& cmd, DWORD dwData)
{
	// Force upper case for lookup.
	std::string asUpper(cmd);
	for (std::string::iterator i = asUpper.begin(); i != asUpper.end(); ++i) {
		*i = ::toupper(*i);
	}

	bool success = false;

	Tug* pTug = pSim->tug();
	if (pTug) {
		success = !pSim->getCommands()->dispatchEvent(asUpper, dwData, pTug->id());
	}
	return success;
}

/// <summary>
/// Determines if the tug (if any) has released
/// </summary>
/// <param name="pSim">Points to the simulator</param>
/// <returns>true if the tug exists and has released</returns>
bool TugMessageHandler::hasReleased(Simulator* pSim)
{
	Tug* pTug = pSim->tug();
	return pTug != 0 && pTug->hasReleased();
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
	if (cmd == "turn") {
		turn(params, output);
	}
	else if (cmd == "wave") {
		waveOff(params, output);
	}
	else if (cmd == "waggle") {
		waggle(params, output);
	}
	else if (cmd == "position") {
		reportPosition(params, output);
	}
	else {
		sendCommand(cmd, params, output);
	}

}
