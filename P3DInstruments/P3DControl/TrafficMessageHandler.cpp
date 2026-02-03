#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "TrafficMessageHandler.h"
#include "Simulator.h"
#include "SimState.h"
#include "APIParameters.h"
#include "Folder.h"
#include "JSONWriter.h"
#include "WideConverter.h"
#include "P3DInstallationDirectory.h"


std::string TrafficMessageHandler::getAircraftTitle(const std::string& path) {

	// Just to confuse things some of the .cfg files are in UTF-16.  Need to look
	// for the Byte Order Mark (BOM) at the start. 
	// See https://docs.microsoft.com/en-gb/windows/win32/intl/using-byte-order-marks
	//title=60ft Pusher Drone

	std::string title;
	std::ifstream ifs(path);

	// Get any BOM characters
	char ch0, ch1;
	ifs >> ch0 >> ch1;

	//std::cout << (int)ch0 << "," << (int)ch1 << std::endl;

	if (-1 == (int)ch0 && -2 == (int)ch1) { // UTF-16
		std::wstring line;
		while (ifs.good()) {
			
			ch0 = ifs.get();  // LS byte
			ch1 = ifs.get();  // MS byte
			wchar_t ch = (wchar_t)((unsigned int)ch1 * 256 + (unsigned int)ch0);
			
			if (ch == '\r') {
				// IGNORE
			}
			else if (ch == '\n') {
				std::wstring wt = line;
				line.erase();

				if (wt.find_first_of(L"title=") == 0) {
					wt = wt.substr(wcslen(L"title="));
					title = ws2s(wt);
					break;
				}

				line.erase(); // start a new line

			}
			else {
				line.push_back(ch);
			}

		}
	}
	else { // normal ascii (hopefully!)
		ifs.seekg(0);
		for (std::string line; std::getline(ifs, line, '\n'); ) {
			if (line.empty()) continue;

			if (line.back() == '\r') line.pop_back();  // windows crlf ?

			// std::cout << line << std::endl;

			if (line.find_first_of("title=") == 0) {
				title = line.substr(strlen("title="));
				break;
			}
		}

	}

	ifs.close();
	std::cout << path << " --> " << title << std::endl;
	return title;
}



TrafficMessageHandler::TrafficMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d, "traffic")
	, pSim(static_cast<Simulator*>(p3d))
{
}

TrafficMessageHandler::~TrafficMessageHandler()
{
}

bool TrafficMessageHandler::launch(Simulator* pSim, const std::string& targetName, const std::string& targetTailNumber, double targetRange, double targetSpeed, double relativeBearing, double relativeTargetHeight)
{
	double R = 6371 / 1.852; // radius of earth in NM (convert km to NM)
	double pi = 3.1415926535897932384636434;

	// Where am I and where am I going?
	SimState::Data currentPosition = pSim->getState()->current();

	// How long to reach me?
	double t = targetRange / targetSpeed;

	// Use these to scale x,y back to lat/long.  Assumes a rectangular grid centred on
	// the current position.  Only valid if distances are small.
	double distPerDegreeLong = 2 * pi * R * cos(currentPosition.Latitude * pi / 180) / 360;
	double distPerDegreeLat = 2 * pi * R / 360;

	// Work out how far we've moved before target gets to our location (over time t).
	double h = currentPosition.Heading * pi / 180; // my heading in radians
	double dx = t * currentPosition.Airspeed * sin(h);
	double dy = t * currentPosition.Airspeed * cos(h);

	// From that position work out where target should start
	double th = (currentPosition.Heading + relativeBearing) * pi / 180;  // reciprocal target heading - go this way to see where target should start;
	dx += targetRange * sin(th);
	dy += targetRange * cos(th);

	// Convert position offsets to nominal lat/long assuming rectangular grid;
	double tLong = currentPosition.Longitude + dx / distPerDegreeLong;
	double tLat = currentPosition.Latitude + dy / distPerDegreeLat;

	SIMCONNECT_DATA_INITPOSITION init;
	init.Airspeed = (DWORD)targetSpeed;
	init.Altitude = currentPosition.Altitude + relativeTargetHeight;
	init.Bank = 0;
	init.Pitch = 0;
	init.Heading = currentPosition.Heading + relativeBearing + 180; // 180 as coming from, not going to
	while (init.Heading >= 360) init.Heading -= 360;
	init.Latitude = tLat;
	init.Longitude = tLong;
	init.OnGround = 0;

	HRESULT hr = ::SimConnect_AICreateNonATCAircraft(
		pSim->getHandle(),
		targetName.c_str(),
		targetTailNumber.c_str(),
		init,
		pSim->nextRequestId()
	);

	return hr == S_OK;
}

std::list<std::string>& TrafficMessageHandler::listAircraft(Simulator* pSim, std::list<std::string>& aircraft)
{

	P3DInstallationDirectory p3dInstall(pSim);
	Directory airplanesFolder = p3dInstall.sub("SimObjects").sub("Airplanes");

	Directory::ListT folders;
	airplanesFolder.folders(folders);

	for (Directory::ListT::iterator it = folders.begin();
		it != folders.end();
		++it) {

		File config = it->file("aircraft.cfg");
		if (!config.exists()) {
			config = it->file("sim.cfg");
		}

		if (config.exists()) {
			std::string title = getAircraftTitle(config);
			aircraft.push_back(title);
		}

	}
	
	return aircraft;
}

void TrafficMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "launch") {
		double targetRange = params.getDouble("range",4.0);  // Start target 4NM away 
		double targetSpeed = params.getDouble("speed",300); // in kts;
		double relativeBearing = params.getDouble("bearing",0);  // degrees 0 is on the nose, 90 from the right etc.
		std::string targetName = params.getString("name"); // one of the aircraft in F:\Lockheed Martin\Prepar3D v4\SimObjects\Airplanes, title from aircraft.cfg
		if (targetName.empty()) targetName = "F-15";
		std::string targetTailNumber = params.getString("tail_number");
		if (targetTailNumber.empty()) targetTailNumber = "Viper";
		double relativeTargetHeight = params.getDouble("relative_height",0); // same level

		launch(targetName, targetTailNumber, targetRange, targetSpeed, relativeBearing, relativeTargetHeight);
		reportSuccess(output);
	}
	else if (cmd == "aircraft") {
		availableAircraft(output);
	}
	else {
		reportFailure("Unknown traffic command", 0, output);
	}
}

void TrafficMessageHandler::launch(const std::string& targetName, const std::string& targetTailNumber, double targetRange, double targetSpeed, double relativeBearing, double relativeTargetHeight)
{
	launch(pSim, targetName, targetTailNumber, targetRange, targetSpeed, relativeBearing, relativeTargetHeight);
}

// Note that if there are multiple title fields in a config file then this
// will only return the first.  Should be fine for traffic.
void TrafficMessageHandler::availableAircraft(std::string& output)
{
	try {

		JSONWriter json(output);
		json.add("status", "OK");
		json.array("aircraft");

		std::list<std::string> aircraft;
		listAircraft(pSim, aircraft);
		for (auto it = aircraft.begin(); it != aircraft.end(); ++it) {
			json.object();
			json.add("title", *it);
			json.end();

		}
		json.end(); // of array

	}
	catch (FileException& fx) {
		output.erase();
		reportFailure(fx.what(), fx.err(), output);
	}
}

