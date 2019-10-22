#include "StdAfx.h"
#include <iostream>
#include <sstream>
#include "LocationData.h"
#include "NMEAData.h"
#include "Transport.h"

SimObjectData::DataItem LocationData::dataItems[] = {
	{"PLANE LATITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE","m",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS POSITION LAT","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS POSITION LON","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS POSITION ALT","m",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS MAGVAR","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS GROUND SPEED","knots",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS GROUND TRUE HEADING","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS GROUND MAGNETIC TRACK","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS GROUND TRUE TRACK","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"AIRSPEED INDICATED", "kph", SIMCONNECT_DATATYPE_FLOAT64},
	{"PRESSURE ALTITUDE","m",SIMCONNECT_DATATYPE_FLOAT64},
	{"VARIOMETER RATE","m/s",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE HEADING DEGREES TRUE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"AMBIENT WIND VELOCITY","kph",SIMCONNECT_DATATYPE_FLOAT64},
	{"AMBIENT WIND DIRECTION","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU TIME","seconds",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU DAY OF MONTH","number",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU MONTH OF YEAR","number",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU YEAR","number",SIMCONNECT_DATATYPE_FLOAT64},

};

SimObjectData::DataItem* LocationData::items() {
	return dataItems;
}

int LocationData::itemCount() {
	return sizeof(dataItems)/sizeof(dataItems[0]);
}

void LocationData::onData(void *pData, SimObject* pObject) {

	struct Data data = *reinterpret_cast<Data*>(pData);
	if(getSim()->isVerbose()) {
		std::cout << "Location Data received" << std::endl;
		// show(pData);
	}
	
	// If no GPS data try to simulate from other data as much as possible.
	if(data.gpsLatitudeDegrees == 0.0 && data.gpsLongditudeDegrees == 0.0) {
		data.gpsLatitudeDegrees = data.latiudeDegrees;
		data.gpsLongditudeDegrees = data.londitudeDegrees;
		data.gpsAltitudeMetres = data.altitudeMetres;
		data.gpsGroundMagneticTrackDegrees = data.planeHeadingDegreesTrue;
		data.gpsGroundTrueHeadingDegrees = data.planeHeadingDegreesTrue;
	} 

	if(pNMEA != 0 && pTransport != 0) {
		// Only bother sending if previous data sent otherwise just ignore
		// We don't want buffering of large amounts of data so this effectively
		// throttles output.
		if(pTransport->canSend()) {
			pNMEA->latDegrees = data.gpsLatitudeDegrees;
			pNMEA->longDegrees = data.gpsLongditudeDegrees;
			pNMEA->gpsHeight = data.gpsAltitudeMetres;
			pNMEA->	utcTime = data.zuluTimeSeconds;
			pNMEA->groundSpeedKnots = data.gpsGroundSpeedKnots;
			pNMEA->trackDegrees = data.gpsGroundTrueTrackDegrees;
			pNMEA->magneticVariationDegrees = data.gpsMagneticVariationDegrees;
			pNMEA->IASkph = data.airspeedIndicatedKph;
			pNMEA->baroAltitudeMetres = data.pressureAltitudeMetres;
			pNMEA->varioMetresSec = data.variometerRateMetres;
			pNMEA->headingDegrees = data.gpsGroundTrueHeadingDegrees;
			pNMEA->windHeadingDegrees = data.ambientWindDirectionDegrees;
			pNMEA->windSpeedKph = data.ambientWindVelocityKph;
			pNMEA->year = data.zuluYear;
			pNMEA->month = data.zuluMonthOfYear;
			pNMEA->day = data.zuluDayOfMonth;

			std::ostringstream oss;
			pNMEA->send(oss);

			std::cout << "Sending: " << std::endl << oss.str() << std::endl;
			pTransport->send(oss.str());
		} else {
			std::cout << "Skipping" << std::endl;
		}
	}
}

LocationData::LocationData(Prepar3D* pSim)
	: SimObjectData(pSim)
	, pNMEA(0)
	, pTransport(0)
{
	createDefinition();
}


LocationData::~LocationData(void)
{
}
