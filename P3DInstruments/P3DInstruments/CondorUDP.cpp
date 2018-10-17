#include "stdafx.h"
#include "CondorUDP.h"
#include <sstream>
 
#pragma comment(lib,"ws2_32.lib") //Winsock Library


CondorUDP::CondorUDP(const char* host, int port)
	: UDPClient(host, port)
	, time(12)
	, airspeed(0)
	, altitude(0)
	, vario(0)
	, compass(0)
	, slipball(0)
	, turnrate(0)
	, yawstringangle(0)
	, yaw(0)
	, pitch(0)
	, bank(0)
	, gforce(1.0)

{
	::strncpy_s(radiofrequency, sizeof(radiofrequency), "131.275", _TRUNCATE);
}


CondorUDP::~CondorUDP(void)
{
}


void CondorUDP::send() {
 
	std::ostringstream os;

	const char* cr = "\n";

	os << "time=" << time << cr;
	os << "airspeed=" << airspeed << cr;
	os << "altitude=" << altitude << cr;
	os << "vario=" << vario << cr;
	os << "compass=" << compass << cr;
	os << "slipball=" << slipball << cr;
	os << "turnrate=" << turnrate << cr;
	os << "yawstringangle=" << yawstringangle << cr;
	os << "radiofrequency=" << radiofrequency << cr;
	os << "yaw=" << yaw << cr;
	os << "pitch=" << pitch << cr;
	os << "bank=" << bank << cr;
	os << "g=" << gforce << cr;

	std::string str = os.str();

    //send the message
	UDPClient::send(str);
         
}