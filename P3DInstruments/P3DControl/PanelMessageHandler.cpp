#include "stdafx.h"
#include "APIParameters.h"
#include "../P3DCommon/UDPClient.h"
#include "PanelMessageHandler.h"


PanelMessageHandler::PanelMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d,"panel")
{
}

PanelMessageHandler::~PanelMessageHandler()
{
}

void PanelMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "send") {
		std::string host = params.getString("host");
		int port = params.getInt("port", 55278);
		std::string cmd = params.getString("cmd");

		if (host.empty() || cmd.empty()) {
			reportFailure("Must supply both host and cmd to send data to panel", 0, output);
		}
		send(host, port, cmd, output);
	}
	else {
		reportFailure("Unknown panel command ", 0, output);
	}

}

void PanelMessageHandler::send(const std::string& host, int port, const std::string& cmd, std::string& output)
{
	UDPClient udp(host.c_str(), port);
	udp.send(cmd);
	reportSuccess(output);
}

