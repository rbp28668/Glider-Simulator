#pragma once
#include <ctype.h>
#include "Spline.h"

// Classes structs to manage a Trace independently of the original source of data. 
// Source data is an array of Point provided by a TraceProvider.  The Trace
// will then provide a Position at time T starting at zero.
// Note use of double - for resolution of a cm or less using lat/long float
// starts to run out of bits and the movement is jerky.

struct Point {
	time_t when;
	double x;  // Longitude in degrees
	double y;  // Latitude in degrees
	double z;  // altitude in feet
};

struct Position {
	double lat;		// degrees
	double lon;		// degrees
	double alt;
	double pitch;  // radians
	double bank;   // radians
	double speed;
	double heading;
	double vwx; // world speed in EW direction
	double vwy; // world speed in NS direction
	double vwz; // world speed height
};

struct TraceProvider {
	virtual size_t count() const = 0;
	virtual Point valueAt(size_t position) const = 0;
};


class Trace
{
	Spline<double>* xSpline; // Longitude in degrees
	Spline<double>* ySpline; // latitude in degrees
	Spline<double>* zSpline; // height in feet

	double* xx; // Longitude base information
	double* yy; // Latitude base information
	double* zz; // altitude base information
	double* tt; // time in seconds for each xx,yy,zz point

	double maxTime; // longest time trace is valid for.

	Position last;

public:
	void build(const TraceProvider& trace);
	Position positionAtTime(double t);

	bool complete(double t) const { return t > maxTime; }

	Trace();
	~Trace();
};

