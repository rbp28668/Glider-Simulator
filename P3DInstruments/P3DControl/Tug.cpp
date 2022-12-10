#include "stdafx.h"
#include <assert.h>
#include "Tug.h"
#include "Simulator.h"
#include "P3DEventCommand.h"
#include "../P3DCommon/SimObjectDataRequest.h"


// Data from the sim, Input to the simulation aircraft
// Needs to match TugInputData structure.
static SimObjectData::DataItem InputStateItems[] = {
	{"PLANE LATITUDE", "degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE", "degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE PITCH DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE BANK DEGREES", "radians", SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE HEADING DEGREES TRUE", "degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{"AI DESIRED SPEED", "knots", SIMCONNECT_DATATYPE_FLOAT64},
	{"AI DESIRED HEADING", "degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{"TOW CONNECTION","bool", SIMCONNECT_DATATYPE_INT32 },
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

/// <summary>
/// Creates a new tug object with the given object ID.
/// The object ID will have been passed to the ObjectCreated event.
/// </summary>
/// <param name="pSim">Pointer to the main simulator instance</param>
/// <param name="dwObjectId">Object ID of the tug plane passed when the object was created.</param>
Tug::Tug(Simulator* pSim, DWORD dwObjectId)
	: SimObject(pSim)
	, pSim(pSim)
	,pInputRequest(0)
	, inputData(p3d, InputStateItems)
	, outputData(p3d, OutputStateItems)
	, controlData(p3d, ControlPositionItems)
	,headingSet(false)
	,speedSet(false)
	,isReleased(false)
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

/// <summary>
/// Deletes the tug and stops any input data being sent to it.
/// </summary>
Tug::~Tug()
{
	if (pInputRequest) {
		pInputRequest->stop();
		p3d->unregisterDataRequest(pInputRequest);
	}
}

/// <summary>
/// Main tug simulation method that is called whenever the tug receives data. 
/// This should be every frame and is triggred by the data request sent when
/// the tug objct is created.
/// </summary>
/// <param name="inputData">Input data describing tug's position, speed, etc.</param>
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

	bool sendAIValues = true;
	if (currentFeature) {
		sendAIValues = currentFeature->tick(this, inputValues, outputValues);
		if (currentFeature->isComplete()) {
			delete currentFeature;
			currentFeature = 0;
		}
	}

	if ( (headingSet || speedSet) && sendAIValues) {
		outputData.send(&outputValues, sizeof(outputValues), id());
	}

}

void Tug::setDesiredSpeed(double kts) {
	outputValues.desiredSpeedKnots = kts;
	speedSet = true;

	if (!headingSet) {
		setDesiredHeading(inputValues.desiredHeading);
	}
}

void Tug::setDesiredHeading(double degrees) {
	outputValues.desiredHeading = degrees;
	headingSet = true;

	if (!speedSet) {
		setDesiredSpeed(inputValues.desiredSpeedKnots);
	}

}

void Tug::turn(double degrees) {
	if (headingSet) {
		outputValues.desiredHeading += degrees;
	}
	else {
		setDesiredHeading(inputValues.desiredHeading + degrees);
	}
}

/// <summary>
/// Sends an event/command to the tug.
/// </summary>
/// <param name="event">is the event ID </param>
/// <param name="dwData">is any parameter for the event</param>
void Tug::dispatchEvent(P3DEventCommand::EventID event, DWORD dwData)
{
	pSim->getCommands()->dispatchEvent(event, dwData, id());
}

/// <summary>
/// Sends and event/command to the tug
/// </summary>
/// <param name="event">if the name of the event</param>
/// <param name="dwData">is any parameter for the event.</param>
void Tug::dispatchEvent(const std::string& event, DWORD dwData)
{
	pSim->getCommands()->dispatchEvent(event, dwData, id());
}

/// <summary>
/// Sets the control positions 
/// </summary>
/// <param name="aileron"></param>
/// <param name="bank"></param>
/// <param name="rudder"></param>
void Tug::setControlPositions(double aileron, double bank, double rudder)
{
	ControlPositionT pos;
	pos.aileron = aileron;
	pos.bank = bank;
	pos.rudder = rudder;

	controlData.send(&pos, sizeof(pos), id());
}

/// <summary>
/// Starts the wing waggle feature
/// </summary>
void Tug::waggleWings()
{
	if (currentFeature == 0) {
		currentFeature = new Tug::WingWaggleFeature();
		currentFeature->start(this, inputValues);
	}
}

/// <summary>
/// Starts the waggle rudder feature.
/// </summary>
void Tug::waggleRudder()
{
	if (currentFeature == 0) {
		currentFeature = new Tug::RudderWaggleFeature();
		currentFeature->start(this, inputValues);
	}
}

/// <summary>
/// Called when the tug is released. Makes it turn away.  
/// See SimState for logic that triggers this when the TOW CONNECTION
/// variable returns false on the user aircraft.
/// </summary>
void Tug::released()
{
	double hdg = inputValues.heading;
	hdg = (hdg < 180) ? hdg + 180 : hdg - 180;
	setDesiredHeading(hdg);
	isReleased = true;
}

/// <summary>
/// Determines the time the tug has been operating in seconds.
/// </summary>
/// <returns></returns>
double Tug::runTime() const
{
	LARGE_INTEGER now;
	::QueryPerformanceCounter(&now);
	double dt = (double)(now.QuadPart - t0.QuadPart);
	return dt / frequency;
}

/// <summary>
/// Constructor for input data.
/// </summary>
/// <param name="pTargetSim">Is the instance of P3D</param>
/// <param name="pItems">Is the data definition for the input data</param>
TugInputData::TugInputData(Prepar3D* pTargetSim, DataItem* pItems)
	: SimObjectData(pTargetSim, pItems)
{
}

/// <summary>
/// Receives the input data from P3D giving current status about the tug.  This 
/// then calls Tug::simulate to process the input data.
/// </summary>
/// <param name="pData">Pointer to the input data (as void*)</param>
/// <param name="pObject">Pointer to the simulation object receiving the data.</param>
void TugInputData::onData(void* pData, SimObject* pObject)
{
	Tug* pTug = static_cast<Tug*>(pObject);
	TugInputDataT* pSimInput = static_cast<TugInputDataT*>(pData);
	pTug->simulate(pSimInput);
}

/// <summary>
/// 
/// </summary>
Tug::RudderWaggleFeature::RudderWaggleFeature()
	: complete(false)
{
}

/// <summary>
/// 
/// </summary>
/// <param name="pTug"></param>
/// <param name="initialState"></param>
void Tug::RudderWaggleFeature::start(Tug* pTug, TugInputDataT& initialState)
{
	startTime = pTug->runTime();
	//pTug->dispatchEvent(P3DEventCommand::FREEZE_ATTITUDE_SET, 1);
}

/// <summary>
/// 
/// </summary>
/// <param name="pTug"></param>
/// <param name="input"></param>
/// <param name="output"></param>
/// <returns></returns>
bool Tug::RudderWaggleFeature::tick(Tug* pTug, const TugInputDataT& input, TugOutputDataT& output)
{
	double t = pTug->runTime() - startTime;
	complete = t > 8;

	// Complete cycle in 2 seconds.
	double rudder = complete ? 0 : 0.5 * sin(t * 3.1415926535);  // (pi radians per second

	// 0.5 radians about 30 degrees
	pTug->setControlPositions(0, 0, rudder);

	// Complete cycle in 2 seconds.
	DWORD dwData = complete ? 0 : (DWORD)(16383 * sin(t * 3.1415926535));  // (pi radians per second

	pTug->dispatchEvent(P3DEventCommand::RUDDER_SET, dwData);


	//if (complete) {
	//	pTug->dispatchEvent(P3DEventCommand::FREEZE_ATTITUDE_SET, 0);
	//}

	return false;
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
bool Tug::RudderWaggleFeature::isComplete()
{
	return complete;
}

/// <summary>
/// 
/// </summary>
Tug::WingWaggleFeature::WingWaggleFeature()
	: complete(false)
	, startTime(0)
	, initalBank(0)
{
}

/// <summary>
/// 
/// </summary>
/// <param name="pTug"></param>
/// <param name="initialState"></param>
void Tug::WingWaggleFeature::start(Tug* pTug, TugInputDataT& initialState)
{
	startTime = pTug->runTime();
	initalBank = pTug->getBankAngle();
	pTug->dispatchEvent(P3DEventCommand::FREEZE_ATTITUDE_SET, 1);
}

/// <summary>
/// 
/// </summary>
/// <param name="pTug"></param>
/// <param name="input"></param>
/// <param name="output"></param>
/// <returns></returns>
bool Tug::WingWaggleFeature::tick(Tug* pTug, const TugInputDataT& input, TugOutputDataT& output)
{
	double t = pTug->runTime() - startTime;
	complete = t > 8;

	// Complete cycle in 2 seconds.
	double aileron = complete ? 0 : cos(t * 3.1415926535);  // (pi radians per second
	double bank = complete ? initalBank : initalBank + (0.5 *sin(t * 3.1415926535));  // (pi radians per second

	// 0.5 radians about 30 degrees
	pTug->setControlPositions(aileron, bank, 0);

	if (complete) {
		pTug->dispatchEvent(P3DEventCommand::FREEZE_ATTITUDE_SET, 0);
	}
	return false;

}

/// <summary>
/// Determine whether the wing waggling feature is complete.
/// </summary>
/// <returns>true if wing waggling is complete</returns>
bool Tug::WingWaggleFeature::isComplete()
{
	return complete;
}
