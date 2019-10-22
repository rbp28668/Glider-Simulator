#include "stdafx.h"
#include <iostream>
#include <limits>
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
	// Note IGC files work in metres but the sim works in feet
	// so altitude converted below so we don't need to convert
	// all the time.
	TracePoint p = points[position];
	Point point;
	point.when = p.getWhen();
	point.x = p.getEastings();
	point.y = p.getNorthings();
	point.z = p.getAltBaro() * 3.28084; // to feet
	return point;
}

// Just sometimes the preprocessor is a bit of a nuisance...
// See numeric_limits below.
#ifdef max
#undef max
#endif

// Calculate minimal altitude in the trace.  IGC files use a 1013hPa reference
// for zero so actual zero on the day may change, as might the elevation of where
// you start from from site to site.
double IGCAircraft::getBaseAltitude(IGCFile* igc)
{
	double minAlt = std::numeric_limits<double>::max();

	for (IGCFile::TraceList::iterator iter = igc->getTrace().begin();
		iter != igc->getTrace().end();
		++iter) {
		double alt = iter->getAltBaro();
		if (alt == 0) {
			alt = iter->getAltGps();
		}
		if (alt < minAlt) {
			minAlt = alt;
		}
	}

	return minAlt * 3.28084; // Convert to feet
}

IGCAircraft::IGCAircraft(IGCFile* igc)
	: baseAltitude(0)
{
	IGCTraceConverter convert(igc);
	trace.build(convert);
	baseAltitude = getBaseAltitude(igc);
}

// Simulate the aircraft at time t.  True if still running, false if complete.
bool IGCAircraft::simulate(const SimInputData& inputData, SimOutputData& outputData, double t)
{
	double PI = 3.1415926535897926364338;

	if (!initialGroundHeightSet) {
		initialGroundHeight = inputData.groundAltitude * 3.28084; // metres to feet
		initialGroundHeight += inputData.staticCGToGround; // try and put it on its wheels/skid...
		initialGroundHeightSet = true;
	}
	Position p = trace.positionAtTime(t);
	outputData.latitude = p.lat * PI / 180;  // radians
	outputData.longitude = p.lon * PI / 180; // radians
	outputData.altitude = p.alt - baseAltitude + initialGroundHeight; 
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

	return !trace.complete(t);
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


/*
UserIGCAircraft
*/

UserIGCAircraft::UserIGCAircraft(Prepar3D* p3d,IGCFile* igc, const char* containerName)
	: ExternalSimVehicle(p3d, containerName)
	, IGCAircraft(igc)
{
}

bool UserIGCAircraft::simulate(const SimInputData& inputData, SimOutputData& outputData, double t)
{
	return IGCAircraft::simulate(inputData, outputData, t);
}

UserIGCAircraft::~UserIGCAircraft()
{
}

/*
AIIGCAircraft
*/

AIIGCAircraft::AIIGCAircraft(Prepar3D* p3d, IGCFile* igc, const char* containerName)
	: AIExternalSimVehicle(p3d, containerName)
	, IGCAircraft(igc)
{
}

bool AIIGCAircraft::simulate(const SimInputData& inputData, SimOutputData& outputData, double t)
{
	return IGCAircraft::simulate(inputData, outputData, t);
}

SIMCONNECT_DATA_INITPOSITION AIIGCAircraft::initialPosition()
{
	return IGCAircraft::initialPosition();
}

AIIGCAircraft::~AIIGCAircraft()
{
}

