#pragma once
#include "../P3DCommon/SimObjectData.h"
#include "MechanicalVario.h"

class CondorUDP;

class InstrumentData :
	public SimObjectData
{
	struct Data {
		double airspeed;
		double tasMetresPerSec;
		double indicatedAltitude;
		double altitudeMetres;
		double kohlsmanMb;
		double pitchAttitude;
		double rollAttitude;
		double wiskeyCompass;
		double headingRate;
		double slipBall;
		__int64 comFrequency;
		double comStatus;
		double gForce;
		double vario;
		double varioSwitch;
		double pressureAltitude;
		double magCompass;
		double turnRate;
		double turnSwitch;
		double yawString;
	} data;

	static DataItem dataItems[];

	CondorUDP* pInstruments;

	MechanicalVario mechanicalVario;

public:

	virtual DataItem* items();
	virtual int itemCount();

	virtual void onData(void *pData);

	void setInstruments(CondorUDP* pInstruments) { this->pInstruments = pInstruments; }

	InstrumentData(Prepar3D*);
	virtual ~InstrumentData(void);
};

