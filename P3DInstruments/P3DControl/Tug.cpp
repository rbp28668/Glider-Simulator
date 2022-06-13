#include "stdafx.h"
#include <assert.h>
#include "Tug.h"
#include "Simulator.h"
#include "P3DEventCommand.h"
#include "../P3DCommon/SimObjectDataRequest.h"


// Data from the sim, Input to the simulation aircraft
static SimObjectData::DataItem InputStateItems[] = {
	{"PLANE LATITUDE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE PITCH DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE BANK DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE HEADING DEGREES TRUE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"AI DESIRED SPEED", "knots", SIMCONNECT_DATATYPE_FLOAT64},
	{"AI DESIRED HEADING", "degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{0,0,SIMCONNECT_DATATYPE_INVALID}
};


// Data back to the sim, Output from the simulation aircraft
static SimObjectData::DataItem OutputStateItems[] = {
	{"AI DESIRED SPEED", "knots", SIMCONNECT_DATATYPE_FLOAT64},
	{"AI DESIRED HEADING", "degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{0,0,SIMCONNECT_DATATYPE_INVALID}
};

// Data back to the sim, Output from the simulation aircraft
static SimObjectData::DataItem ControlPositionItems[] = {
	{"AILERON POSITION", "position", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE BANK DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"RUDDER POSITION", "position", SIMCONNECT_DATATYPE_FLOAT64},
	//{"PLANE HEADING DEGREES TRUE", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{0,0,SIMCONNECT_DATATYPE_INVALID}
};

Tug::Tug(Simulator* pSim, DWORD dwObjectId)
	: SimObject(pSim)
	, pSim(pSim)
	,pInputRequest(0)
	, inputData(p3d, InputStateItems)
	, outputData(p3d, OutputStateItems)
	, controlData(p3d, ControlPositionItems)
	,headingSet(false)
	,speedSet(false)
	,currentFeature(0)
{
	LARGE_INTEGER freq;
	::QueryPerformanceFrequency(&freq);
	frequency = (double)freq.QuadPart;

	::QueryPerformanceCounter(&t0);

	setObjectId(dwObjectId);
	setName("tug");
	
	inputData.createDefinition();
	outputData.createDefinition();
	controlData.createDefinition();

	pInputRequest = new SimObjectDataRequest(p3d, &inputData, this, SIMCONNECT_PERIOD_VISUAL_FRAME);

}

Tug::~Tug()
{
	if (pInputRequest) {
		pInputRequest->stop();
		p3d->unregisterDataRequest(pInputRequest);
	}
}

void Tug::simulate(TugInputDataT* inputData)
{
	inputValues = *inputData;

	// Only send data if values set.
	if (!headingSet) {
		outputValues.desiredHeading = inputData->desiredHeading;
	}
	if (!speedSet) {
		outputValues.desiredSpeedKnots = inputData->desiredSpeedKnots;
	}

	bool featureWritesData = false;
	if (currentFeature) {
		featureWritesData = currentFeature->tick(this, inputValues, outputValues);
		if (currentFeature->isComplete()) {
			delete currentFeature;
			currentFeature = 0;
		}
	}

	if (headingSet || speedSet || featureWritesData) {
		outputData.send(&outputValues, sizeof(outputValues), id());
	}

}

void Tug::dispatchEvent(P3DEventCommand::EventID event, DWORD dwData)
{
	pSim->getCommands()->dispatchEvent(event, dwData, id());
}

void Tug::setControlPositions(double aileron, double bank, double rudder)
{
	ControlPositionT pos;
	pos.aileron = aileron;
	pos.bank = bank;
	pos.rudder = rudder;

	controlData.send(&pos, sizeof(pos), id());
}

void Tug::waggleWings()
{
	if (currentFeature == 0) {
		currentFeature = new Tug::WingWaggleFeature();
		currentFeature->start(this, inputValues);
	}
}

void Tug::waggleRudder()
{
	if (currentFeature == 0) {
		currentFeature = new Tug::RudderWaggleFeature();
		currentFeature->start(this, inputValues);
	}
}

void Tug::released()
{
	double hdg = inputValues.heading;
	hdg = (hdg < 180) ? hdg + 180 : hdg - 180;
	setDesiredHeading(hdg);
}

double Tug::runTime() const
{
	LARGE_INTEGER now;
	::QueryPerformanceCounter(&now);
	double dt = (double)(now.QuadPart - t0.QuadPart);
	return dt / frequency;
}

TugInputData::TugInputData(Prepar3D* pTargetSim, DataItem* pItems)
	: SimObjectData(pTargetSim, pItems)
{
}

void TugInputData::onData(void* pData, SimObject* pObject)
{
	Tug* pTug = static_cast<Tug*>(pObject);
	TugInputDataT* pSimInput = static_cast<TugInputDataT*>(pData);
	pTug->simulate(pSimInput);
}

Tug::RudderWaggleFeature::RudderWaggleFeature()
	: complete(false)
{
}

void Tug::RudderWaggleFeature::start(Tug* pTug, TugInputDataT& initialState)
{
	startTime = pTug->runTime();
}

bool Tug::RudderWaggleFeature::tick(Tug* pTug, const TugInputDataT& input, TugOutputDataT& output)
{
	double t = pTug->runTime() - startTime;
	complete = t > 8;

	// Complete cycle in 2 seconds.
	DWORD dwData = complete ? 0 : (DWORD)(16383 * sin(t * 3.1415926535));  // (pi radians per second

	pTug->dispatchEvent(P3DEventCommand::RUDDER_SET, dwData);

	return false;
}

bool Tug::RudderWaggleFeature::isComplete()
{
	return complete;
}

Tug::WingWaggleFeature::WingWaggleFeature()
	: complete(false)
{
}

void Tug::WingWaggleFeature::start(Tug* pTug, TugInputDataT& initialState)
{
	startTime = pTug->runTime();
	pTug->dispatchEvent(P3DEventCommand::FREEZE_ATTITUDE_SET, 1);
}

bool Tug::WingWaggleFeature::tick(Tug* pTug, const TugInputDataT& input, TugOutputDataT& output)
{
	double t = pTug->runTime() - startTime;
	complete = t > 8;

	// Complete cycle in 2 seconds.
	double aileron = complete ? 0 : cos(t * 3.1415926535);  // (pi radians per second
	double bank = complete ? 0 : 0.5 *sin(t * 3.1415926535);  // (pi radians per second

	// 0.5 radians about 30 degrees
	pTug->setControlPositions(aileron, bank, 0);

	if (complete) {
		pTug->dispatchEvent(P3DEventCommand::FREEZE_ATTITUDE_SET, 0);
	}
	return false;

}

bool Tug::WingWaggleFeature::isComplete()
{
	return complete;
}
