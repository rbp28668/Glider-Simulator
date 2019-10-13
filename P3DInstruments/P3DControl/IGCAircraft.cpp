#include "stdafx.h"
#include <iostream>
#include "IGCAircraft.h"
#include "IGCFile.h"
#include "..\\P3DCommon\ExternalSim.h"


class IGCTraceConverter : public TraceProvider {
	IGCFile::TraceList& points;
public:
	virtual size_t count() const { return points.size(); }
	virtual Point valueAt(size_t position) const;
	IGCTraceConverter(IGCFile* igc) : points(igc->getTrace()) {}
};

Point IGCTraceConverter::valueAt(size_t position) const
{
	TracePoint p = points[position];
	Point point;
	point.when = p.getWhen();
	point.x = p.getEastings();
	point.y = p.getNorthings();
	point.z = p.getAltBaro();
	return point;
}



IGCAircraft::IGCAircraft()
{
}

void IGCAircraft::simulate(const SimInputData& inputData, SimOutputData& outputData, double t)
{
	double PI = 3.1415926535897926364338;

	Position p = trace.positionAtTime(t);
	outputData.latitude = p.lat * PI / 180;  // radians
	outputData.longitude = p.lon * PI / 180; // radians
	outputData.altitude = p.alt + 255; // BODGE of 255.
	outputData.bank = p.bank;
	outputData.pitch = p.pitch;
	outputData.heading = p.heading;
	outputData.vBodyZ = 0;
	outputData.vBodyX = 0; // lateral speed relative to aircraft axis;
	outputData.vBodyY = 0; // vertical speed wrt aircraft axis;
	outputData.vWorldX = 0; // p.vwx; // in east/west
	outputData.vWorldZ = 0; // p.vwy; // North/South
	outputData.vWorldY = 0; // p.vwz; // up/down
	outputData.verticalSpeed = 0; // p.vwz;

	//std::cout << "Lat: " << p.lat << "Long: " << p.lon << "Alt (feet)" << p.alt << std::endl;
}

SIMCONNECT_DATA_INITPOSITION IGCAircraft::initialPosition()
{
	Position p = trace.positionAtTime(0);
	SIMCONNECT_DATA_INITPOSITION init;
	init.Latitude = p.lat;
	init.Longitude = p.lon;
	init.Altitude = p.alt;
	init.Airspeed = p.speed;
	init.Bank = p.bank;
	init.Pitch = p.pitch;
	init.Heading = p.heading;
	init.OnGround = true;
	return init;
}


UserIGCAircraft::UserIGCAircraft(Prepar3D* p3d,IGCFile* igc, const char* containerName)
	: ExternalSimVehicle(p3d, containerName)
{
}

void UserIGCAircraft::simulate(const SimInputData& inputData, SimOutputData& outputData, double t)
{
	IGCAircraft::simulate(inputData, outputData, t);
}

UserIGCAircraft::~UserIGCAircraft()
{
}

AIIGCAircraft::AIIGCAircraft(Prepar3D* p3d, IGCFile* igc, const char* containerName)
	: AIExternalSimVehicle(p3d, containerName)
{
	IGCTraceConverter convert(igc);
	trace.build(convert);
}

void AIIGCAircraft::simulate(const SimInputData& inputData, SimOutputData& outputData, double t)
{
	IGCAircraft::simulate(inputData, outputData, t);
}

SIMCONNECT_DATA_INITPOSITION AIIGCAircraft::initialPosition()
{
	return IGCAircraft::initialPosition();
}

AIIGCAircraft::~AIIGCAircraft()
{
}

