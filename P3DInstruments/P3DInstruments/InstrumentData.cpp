#include "StdAfx.h"
#include "InstrumentData.h"
#include "CondorUDP.h"

SimObjectData::DataItem InstrumentData::dataItems[] = {
	{ "AIRSPEED INDICATED", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{ "AIRSPEED TRUE", "m/s", SIMCONNECT_DATATYPE_FLOAT64},
	{ "INDICATED ALTITUDE", "Feet", SIMCONNECT_DATATYPE_FLOAT64},
	{ "PLANE ALTITUDE","m", SIMCONNECT_DATATYPE_FLOAT64},
	{ "KOHLSMAN SETTING MB", "Millibars", SIMCONNECT_DATATYPE_FLOAT64},
	{ "ATTITUDE INDICATOR PITCH DEGREES", "Radians", SIMCONNECT_DATATYPE_FLOAT64},
	{ "ATTITUDE INDICATOR BANK DEGREES","Radians", SIMCONNECT_DATATYPE_FLOAT64},
	{ "WISKEY COMPASS INDICATION DEGREES","Degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{ "DELTA HEADING RATE", "Radians per second", SIMCONNECT_DATATYPE_FLOAT64},
	{ "TURN COORDINATOR BALL", "Position 128", SIMCONNECT_DATATYPE_FLOAT64},
	{ "COM ACTIVE FREQUENCY:1", "Frequency BCD32",SIMCONNECT_DATATYPE_FLOAT64}, 
	{ "COM STATUS:1", "Bool",SIMCONNECT_DATATYPE_FLOAT64}, 
	{ "G FORCE", "GForce",  SIMCONNECT_DATATYPE_FLOAT64}, 
	{ "VARIOMETER RATE", "knots", SIMCONNECT_DATATYPE_FLOAT64}, 
	{ "VARIOMETER SWITCH", "Bool", SIMCONNECT_DATATYPE_FLOAT64},
	{ "PRESSURE ALTITUDE", "Feet", SIMCONNECT_DATATYPE_FLOAT64},
	{ "MAGNETIC COMPASS", "Degrees", SIMCONNECT_DATATYPE_FLOAT64},
	{ "TURN INDICATOR RATE", "Radians per second", SIMCONNECT_DATATYPE_FLOAT64},
	{ "TURN INDICATOR SWITCH", "Bool", SIMCONNECT_DATATYPE_FLOAT64},
	{ "YAW STRING ANGLE", "Radians", SIMCONNECT_DATATYPE_FLOAT64},
};

/*
AIRSPEED INDICATED: 62.727766 m/s
INDICATED ALTITUDE: 181.227066 Feet
KOHLSMAN SETTING MB: 1013.099976 Millibars
ATTITUDE INDICATOR PITCH DEGREES: -0.134878 Radians
ATTITUDE INDICATOR BANK DEGREES: 0.817284 Radians
WISKEY COMPASS INDICATION DEGREES: 263.327026 Degrees
DELTA HEADING RATE: -0.114216 Radians per second
TURN COORDINATOR BALL: 0.000000 Position 128
COM ACTIVE FREQUENCY:1: 19410944.000000 Frequency BCD32
COM STATUS:1: 0.000000 Bool
G FORCE: 1.982452 GForce
VARIOMETER RATE: 0.000000 knots
VARIOMETER SWITCH: 1.000000 Bool
PRESSURE ALTITUDE: 186.665207 Feet
MAGNETIC COMPASS: 263.327026 Degrees
TURN INDICATOR RATE: -0.114216 Radians per second
TURN INDICATOR SWITCH: 1.000000 Bool
YAW STRING ANGLE: 0.000000 Radians
*/

SimObjectData::DataItem* InstrumentData::items() {
	return dataItems;
}

int InstrumentData::itemCount() {
	return sizeof(dataItems)/sizeof(dataItems[0]);
}


void InstrumentData::onData(void *pData) {
	data = *reinterpret_cast<Data*>(pData);
	////printf("IAS %f Knots\n",data.airspeed);
	//printf("Frequency: %16.16lx (%16.16lx)\n", data.comFrequency, data.comStatus);
	////printf("Wiskey Compass: %f\n",data.wiskeyCompass);
	////printf("Mag Compass: %f\n",data.magCompass);
	//printf("Slip ball %ld\n",data.slipBall);
	//printf("Turn rate %f\n",data.turnRate);
	//printf("Vario %f\n",data.vario);
	//printf("G %f\n",data.gForce);
	//printf("Spare 1: %f\n",data.spare1);
	//printf("Spare 2: %f\n",data.spare2);
	
	show(pData);

	if(pInstruments != 0) {
		pInstruments->airspeed = data.airspeed;
		pInstruments->altitude = data.indicatedAltitude;
		pInstruments->bank = data.rollAttitude;
		pInstruments->compass = data.wiskeyCompass; // data.magCompass;?
		pInstruments->gforce = data.gForce;
		pInstruments->pitch = data.pitchAttitude;

		//Frequency is a BCD number.  Frequency: 0000000001283000 is 128.30 MHz
		//Unpack into a string with the decimal point in the appropriate place.
		char f[8];
		int idx = 0;
		long freqBCD = (data.comFrequency >> 32);

		f[idx++] = '1'; //'0' + ((freqBCD & 0x0F000000) >> 24);
		f[idx++] = '0' + (char)((freqBCD & 0x000F0000) >> 16);
		f[idx++] = '0' + (char)((freqBCD & 0x0000F000) >> 12);
		f[idx++] = '.';
		f[idx++] = '0' + (char)((freqBCD & 0x00000F00) >> 8);
		f[idx++] = '0' + (char)((freqBCD & 0x000000F0) >> 4);
		f[idx++] = 0;

		strncpy_s(pInstruments->radiofrequency, sizeof(pInstruments->radiofrequency),f,_TRUNCATE);
		pInstruments->slipball = data.slipBall / 128.0;
		pInstruments->time = 12.0; // TODO: time
		pInstruments->turnrate = -data.turnRate;  // condor uses opposite sense
		pInstruments->vario = data.vario;

		DWORD now = ::GetTickCount();
		pInstruments->vario = mechanicalVario.update(data.altitudeMetres, data.tasMetresPerSec, now);
		 

		pInstruments->send();
	}
}


InstrumentData::InstrumentData(Prepar3D* pSim)
	: SimObjectData(pSim)
	, pInstruments(0)
{
	createDefinition();
}


InstrumentData::~InstrumentData(void)
{
}
