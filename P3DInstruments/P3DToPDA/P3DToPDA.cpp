// P3DToPDA.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../P3DCommon/Prepar3D.h"
#include "../P3DCommon/SimObjectDataRequest.h"
#include "LocationData.h"
#include "NMEAData.h"
#include "SerialLink.h"
#include "UDPTransport.h"
#include "TCPTransport.h"

enum ConnectionT {
	SERIAL,
	UDP,
	TCP
};

int main(int argc, char* argv[])
{
	// Parameters for command line
	enum ConnectionT connectionType = SERIAL;
	char* serialPort = "COM1";
	char* host = "127.0.0.1";
	int port = 4353;
	DWORD delay = 500;
	bool verbose = false;

	for(int i=1; i<argc; ) {
		std::string arg = argv[i++];
		if(arg == "-serial") {
			connectionType = SERIAL;
			if(i < argc) {
				serialPort = argv[i++];
			} else {
				std::cerr << "Missing port parameter for -serial option, defaulting to " << serialPort << std::endl;
			}
			
		} else if (arg == "-tcp") {
			connectionType = TCP;
			if(i < argc) {
				host = argv[i++];
			} else {
				std::cerr << "Missing host parameter for -tcp option, defaulting to " << host << std::endl;
			}

			if(i < argc) {
				port = ::atoi(argv[i++]);
			} else {
				std::cerr << "Missing port parameter for -tcp option, defaulting to " << port << std::endl;
			}

		} else if (arg == "-udp") {
			connectionType = UDP;
			if(i < argc) {
				host = argv[i++];
			} else {
				std::cerr << "Missing host parameter for -udp option, defaulting to " << host << std::endl;
			}

			if(i < argc) {
				port = ::atoi(argv[i++]);
			} else {
				std::cerr << "Missing port parameter for -udp option, defaulting to " << port << std::endl;
			}

		} else if (arg == "-delay") {
			if(i < argc) {
				delay = (DWORD) atoi(argv[i++]);
			} else {
				std::cerr << "Missing delay parameter for -delay option, defaulting to " << delay << "mS" << std::endl;
			}
		} else if (arg == "-verbose") {
			verbose = true;
		} else if (arg == "-help" || arg == "?" ) {
			std::cout << "Usage:" << std::endl;
			std::cout << "P3DToPDA [ -serial <com-port> ] [-verbose] [-delay <mS>]" << std::endl;
			std::cout << "P3DToPDA [ -tcp <IP> <port> ] [-verbose] [-delay <mS>]" << std::endl;
			std::cout << "P3DToPDA [ -udp <IP> <port> ] [-verbose] [-delay <mS>]" << std::endl;
			return 1;
		} else {
			std::cerr << "Unrecognised option " << arg << std::endl;
			return 2;
		}

	}



	std::cout << "Starting P3DtoPDA on ";
	switch(connectionType) {
	case SERIAL:
		std::cout << "SERIAL: " << serialPort;
		break;

	case TCP:
		std::cout << "TCP: " << host << ":" << port;
		break;

	case UDP:
		std::cout << "UDP: " << host << ":" << port;
		break;

	default: 
		std::cerr << "Unknown protocol" << connectionType << std::endl;
	}

	std::cout << " with delay " << delay << "mS" << std::endl;

	Prepar3D sim("P3DtoPDA");
	sim.setVerbose(true);

	LocationData data(&sim);
	NMEAData nmea;
	data.setNMEA(&nmea);

	Transport* transport = 0;

	switch(connectionType) {
	case SERIAL:
		transport = new SerialLink(serialPort);
		break;

	case UDP:
		transport = new UDPTransport(host, port);
		((UDPTransport*)transport) -> setDelay(delay);
		break;

	case TCP:
		transport = new TCPTransport(host,port);
		((TCPTransport*)transport) -> setDelay(delay);
		break;

	default:
		std::cerr << "Invalid transport " << connectionType << std::endl;
		exit(1);
	}

	data.setTransport(transport);

	SimObjectDataRequest request(&sim, &data);

	sim.DispatchLoop();

	delete transport;

	return 0;
}

