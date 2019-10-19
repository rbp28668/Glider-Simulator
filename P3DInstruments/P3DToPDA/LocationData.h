#pragma once
#include "..\P3DCommon\simobjectdata.h"

class Prepar3D;
class NMEAData;
class Transport;

class LocationData :
	public SimObjectData
{

	static DataItem dataItems[];

	struct Data {
		double latiudeDegrees;			//{"PLANE LATITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double londitudeDegrees;		//{"PLANE LONGITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double altitudeMetres;			//{"PLANE ALTITUDE","m",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsLatitudeDegrees;		//{"GPS POSITION LAT","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsLongditudeDegrees;	//{"GPS POSITION LON","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsAltitudeMetres;		//{"GPS POSITION ALT","m",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsMagneticVariationDegrees; //{"GPS MAGVAR","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsGroundSpeedKnots;		// {"GPS GROUND SPEED","knots",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsGroundTrueHeadingDegrees; //{"GPS GROUND TRUE HEADING","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsGroundMagneticTrackDegrees; //{"GPS GROUND MAGNETIC TRACK","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double gpsGroundTrueTrackDegrees; //{"GPS GROUND TRUE TRACK","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double airspeedIndicatedKph;	//{"AIRSPEED INDICATED", "kph", SIMCONNECT_DATATYPE_FLOAT64},
		double pressureAltitudeMetres;	//{"PRESSURE ALTITUDE","m",SIMCONNECT_DATATYPE_FLOAT64},
		double variometerRateMetres;	//{"VARIOMETER RATE","m/s",SIMCONNECT_DATATYPE_FLOAT64},
		double planeHeadingDegreesTrue;	//{"PLANE HEADING DEGREES TRUE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double ambientWindVelocityKph;	//{"AMBIENT WIND VELOCITY","kph",SIMCONNECT_DATATYPE_FLOAT64},
		double ambientWindDirectionDegrees; //			{"AMBIENT WIND DIRECTION","degrees",SIMCONNECT_DATATYPE_FLOAT64},
		double zuluTimeSeconds;			//{"ZULU TIME","seconds",SIMCONNECT_DATATYPE_FLOAT64},
		double zuluDayOfMonth;			//{"ZULU DAY OF MONTH","number",SIMCONNECT_DATATYPE_FLOAT64},
		double zuluMonthOfYear;			//{"ZULU MONTH OF YEAR","number",SIMCONNECT_DATATYPE_FLOAT64},
		double zuluYear;				// {"ZULU YEAR","number",SIMCONNECT_DATATYPE_FLOAT64},
	};

	NMEAData* pNMEA;
	Transport* pTransport;
public:

	void setNMEA(NMEAData* pNMEA) {this->pNMEA = pNMEA; }
	void setTransport(Transport* pTransport) {this->pTransport = pTransport;}

	virtual DataItem* items();
	virtual int itemCount();

	virtual void onData(void *pData, SimObject* pObject);

	LocationData(Prepar3D*);
	~LocationData(void);
};

