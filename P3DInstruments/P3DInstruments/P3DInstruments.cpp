// P3DInstruments.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../P3DCommon/Prepar3D.h"
#include "../P3DCommon/SimObjectData.h"
#include "../P3DCommon/SimObjectDataRequest.h"
#include "InstrumentData.h"
#include "CondorUDP.h"

int main(int argc, char* argv[])
{
	char* host = "127.0.0.1";
	int port = 55278; // default port used by Condor
	bool verbose = false;

		for(int i=1; i<argc; ) {
		std::string arg = argv[i++];
		if(arg == "-host") {
			if(i < argc) {
				host = argv[i++];
			} else {
				std::cerr << "Missing host parameter for -host option, defaulting to " << host << std::endl;
			}
		} else if (arg == "-port") {
			if(i < argc) {
				port = ::atoi(argv[i++]);
			} else {
				std::cerr << "Missing port parameter for -port option, defaulting to " << port << std::endl;
			}

		} else if (arg == "-verbose") {
			verbose = true;
		} else if (arg == "-help" || arg == "?" ) {
			std::cout << "Usage:" << std::endl;
			std::cout << "P3DInstruments [ -host <IP> ] [ -port <port> ] [-verbose]" << std::endl;
			return 1;
		} else {
			std::cerr << "Unrecognised option " << arg << std::endl;
			return 2;
		}
	}

	std::cout << "Starting P3DInstruments on " << host << ":" << port << std::endl;
	Prepar3D sim("P3DInstruments");
	sim.setVerbose(verbose);

	InstrumentData data(&sim);
	SimObjectDataRequest request(&sim, &data, &sim.userAircraft());

	CondorUDP udp(host, port);
	data.setInstruments(&udp);

	sim.DispatchLoop();

	return 0;
}

