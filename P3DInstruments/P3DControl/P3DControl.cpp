// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <windows.h> 

#include <SimConnect.h>
#include "../P3DCommon/Prepar3D.h"
#include "../P3DCommon/SimObjectData.h"
#include "../P3DCommon/SimObjectDataRequest.h"
#include "Simulator.h"
#include "MessageThread.h"
#include "EventMessageHandler.h"
#include "HTTPService.h"
#include "APIThread.h"

int main(int argc, char* argv[])
{

	bool verbose = false;
	unsigned short port = 3000;

	for (int i = 1; i<argc; ) {
		std::string arg = argv[i++];
        if (arg == "-port") {
			if (i < argc) {
				port = ::atoi(argv[i++]);
			}
			else {
				std::cerr << "Missing port parameter for -port option, defaulting to " << port << std::endl;
			}

		}
		else if (arg == "-verbose") {
			verbose = true;
		}
		else if (arg == "-help" || arg == "?") {
			std::cout << "Usage:" << std::endl;
			std::cout << "P3DControl [ -port <port> ] [-verbose]" << std::endl;
			return 1;
		}
		else {
			std::cerr << "Unrecognised option " << arg << std::endl;
			return 2;
		}
	}

	HTTPService httpService;


	Simulator sim("P3DControl",verbose);

	CommandInterpreter interpreter(&sim);

	// Start receiver for UDP messages
	MessageThread messageReceiver(&interpreter, port); // listen on port for UDP messages
	messageReceiver.start();

	// Start receiver for REST API
	APIThread api(&interpreter);
	api.start();

	sim.DispatchLoop();
	
	api.stop();
	messageReceiver.stop();

	return 0;
}

