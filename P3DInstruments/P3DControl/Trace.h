#pragma once
#include <ctype.h>
#include "Spline.h"

struct Point {
	time_t when;
	float x;  // Longitude in degrees
	float y;  // Latitude in degrees
	float z;  // altitude in feet
};

struct Position {
	float lat;
	float lon;
	float alt;
	float pitch;
	float bank;
	float speed;
	float heading;
	float vwx; // world speed in EW direction
	float vwy; // world speed in NS direction
	float vwz; // world speed height
};

struct TraceProvider {
	virtual size_t count() const = 0;
	virtual Point valueAt(size_t position) const = 0;
};


class Trace
{
	Spline* xSpline; // Longitude in degrees
	Spline* ySpline; // latitude in degrees
	Spline* zSpline; // height in feet

	float* xx; // Longitude base information
	float* yy; // Latitude base information
	float* zz; // altitude base information
	float* tt; // time in seconds for each xx,yy,zz point

	float maxTime; // longest time trace is valid for.

	Position last;

public:
	void build(const TraceProvider& trace);
	Position positionAtTime(double t);

	Trace();
	~Trace();
};

