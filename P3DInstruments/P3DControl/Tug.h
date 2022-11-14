#pragma once
#include "../P3DCommon/SimObject.h"
#include "../P3DCommon/SimObjectData.h"
#include "P3DEventCommand.h"

class TugInputData;
class Simulator;

struct TugInputDataT {
	double latitude;
	double longitude;
	double altitude;
	double pitch;
	double bank;
	double heading;
	double desiredSpeedKnots;
	double desiredHeading;
	int towAttached;
};


struct TugOutputDataT {
	double desiredSpeedKnots;
	double desiredHeading;
};

struct ControlPositionT {
	double aileron;
	double bank;
	double rudder;
	//double yaw;
};

// Input data that knows to redirect any data requests into the ExternalSim object
class TugInputData : public SimObjectData {
public:
	TugInputData(Prepar3D* pTargetSim, DataItem* pItems);
	virtual void onData(void* pData, SimObject* pObject);
};

/// <summary>
/// Class to control behaviour of the Tug aircraft.
/// </summary>
class Tug :
	public SimObject
{

	/// <summary>
	/// A Feature is some tug behaviour that can be started and may take place over a period of time.
	/// </summary>
	class Feature {
	public:
		virtual ~Feature() {}
		virtual void start(Tug* pTug, TugInputDataT& initialState) = 0;
		virtual bool tick(Tug* pTug, const TugInputDataT & input, TugOutputDataT & output) = 0;
		virtual bool isComplete() = 0;
	};

	/// <summary>
	/// WingWaggleFeature drives a tug's wave-off signal
	/// </summary>
	class WingWaggleFeature : public Feature{
		double startTime;
		bool complete;
	public:
		WingWaggleFeature();
		virtual ~WingWaggleFeature() {}
		virtual void start(Tug* pTug, TugInputDataT& initialState);
		virtual bool tick(Tug* pTug,  const TugInputDataT& input, TugOutputDataT& output);
		virtual bool isComplete();
	};

	/// <summary>
	/// RudderWaggleFeature drives a tug's "you've got a problem e.g. brakes" signal.
	/// Note - currently doesn't work!
	/// </summary>
	class RudderWaggleFeature : public Feature {
		double startTime;
		bool complete;
	public:
		RudderWaggleFeature();
		virtual ~RudderWaggleFeature() {}
		virtual void start(Tug* pTug, TugInputDataT& initialState);
		virtual bool tick(Tug* pTug,  const TugInputDataT& input, TugOutputDataT& output);
		virtual bool isComplete();
	};

	Simulator* pSim;
	TugInputData inputData;
	SimObjectData outputData;
	SimObjectData controlData;

	TugInputDataT inputValues;
	TugOutputDataT outputValues;
	bool headingSet;
	bool speedSet;
	bool isReleased;

	Feature* currentFeature;

	SimObjectDataRequest* pInputRequest; // input request to get data to drive the object.
	LARGE_INTEGER t0;    // when this started.
	double frequency; // of timer in ticks per second.

public:

	Tug(Simulator* pSim, DWORD dwObjectId);
	virtual ~Tug();

	double runTime() const;

	void simulate(TugInputDataT* inputData);
	void dispatchEvent(P3DEventCommand::EventID event, DWORD dwData);
	void dispatchEvent(const std::string& event, DWORD dwData);

	void setDesiredSpeed(double kts) {
		outputValues.desiredSpeedKnots = kts;
		speedSet = true;
	}

	void setDesiredHeading(double degrees) {
		outputValues.desiredHeading = degrees;
		headingSet = true;
	}

	void setControlPositions(double aileron, double bank, double rudder);
	void waggleWings();
	void waggleRudder();
	void released();
	bool hasReleased() { return isReleased; }
	double getAltitude() { return inputValues.altitude; }
	double getHeading() { return inputValues.heading; }
	double getLatitude() { return inputValues.latitude; }
	double getLongitude() { return inputValues.longitude; }
	double getDesiredSpeed() { return inputValues.desiredSpeedKnots; }
	double getDesiredHeading() { return inputValues.desiredHeading; }
};

