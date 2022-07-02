#include "stdafx.h"
#include "..//P3DCommon/Prepar3D.h"
#include "ThermalMessageHandler.h"
#include "Simulator.h"
#include "SimState.h"
#include "APIParameters.h"

ThermalMessageHandler::ThermalMessageHandler(Prepar3D* p3d)
	: MessageHandler(p3d,"thermal")
{
}

bool ThermalMessageHandler::drop(Simulator* pSim, float radius, float height, float coreRate)
{
	SimState* pState = pSim->getState();
	SimState::Data current = pState->current();

	float lat = (float)current.Latitude;
	float lon = (float)current.Longitude;
	return drop(pSim, radius, height, coreRate, lat, lon);
}



bool ThermalMessageHandler::drop(Simulator* pSim, float radius, float height, float coreRate, float lat, float lon)
{
	float alt = 0;  // start at ground
	float coreTurbulence = coreRate * 0.3f;
	float sinkRate = coreRate * 0.5f; // make surrounding sink about 1/2 of core lift
	float sinkTurbulence = sinkRate * 0.3f;

	SIMCONNECT_DATA_REQUEST_ID requestID = pSim->nextRequestId();
	HRESULT hr = ::SimConnect_WeatherCreateThermal(pSim->getHandle(), requestID, lat, lon, alt, radius, height,
		coreRate, coreTurbulence, sinkRate, sinkTurbulence);
	return(hr == S_OK);
}

void ThermalMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	if (cmd == "drop") {
		dropThermal(params, output);
	}
	else {
		reportFailure("Thermal command not recognised", 0, output);
	}
}

void ThermalMessageHandler::dropThermal(const APIParameters& params, std::string& output)
{
	Simulator* pSim = static_cast<Simulator*>(p3d);
	float radius = params.getFloat("radius", 300);
	float height = params.getFloat("height", 2000); // metres
	float coreRate = params.getFloat("rate", 4);  // m/s - ballistic!
	if(drop(pSim, radius, height, coreRate)) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to create thermal", 0, output);
	}
}
