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
#include "MessageThread.h"
#include "EventMessageHandler.h"


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

	Prepar3D sim("P3DControl");
	sim.setVerbose(verbose);

	MessageThread messageReceiver(port); // listen on port for UDP messages
	EventMessageHandler messageHandler(&sim);
	messageReceiver.add(&messageHandler);
	messageReceiver.start();

	sim.DispatchLoop();
	
	messageReceiver.stop();
	messageReceiver.remove(&messageHandler);

	return 0;
}

