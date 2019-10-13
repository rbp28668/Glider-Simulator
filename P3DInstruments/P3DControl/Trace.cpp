#include "stdafx.h"
#include "Trace.h"
#include <math.h>

Trace::Trace()
: xx(0)
, yy(0)
, zz(0)
, tt(0)
, xSpline(0)
, ySpline(0)
, zSpline(0)
, maxTime(0)
{
}

Trace::~Trace()
{
	delete xSpline;
	delete ySpline;
	delete zSpline;

	if (xx) delete[] xx;
	if (yy) delete[] yy;
	if (zz) delete[] zz;
	if (tt) delete[] tt;
}

void Trace::build(const TraceProvider& trace)
{
	int count = trace.count();
	xx = new float[count];
	yy = new float[count];
	zz = new float[count];
	tt = new float[count];

	time_t tStart = trace.valueAt(0).when;

	for (int i = 0; i < count; ++i) {
		Point p = trace.valueAt(i);
		xx[i] = p.x; // long
		yy[i] = p.y; // lat
		zz[i] = p.z; // altitude feet
		tt[i] = p.when - tStart;
	}
	maxTime = trace.valueAt(count - 1).when - tStart;

	xSpline = new Spline(tt, xx, count);
	ySpline = new Spline(tt, yy, count);
	zSpline = new Spline(tt, zz, count);

}

Position Trace::positionAtTime(double t)
{
	if (t < maxTime) {
		// First and second derivatives
		float dxdt, dydt, dzdt;
		float d2xdt2, d2ydt2, d2zdt2;

		// Points at this time.
		float x = xSpline->point(t, dxdt, d2xdt2);
		float y = ySpline->point(t, dydt, d2ydt2);
		float z = zSpline->point(t, dzdt, d2zdt2);

		last.lon = x;
		last.lat = y;
		last.alt = z;

		double R = 6371 / 1.852; // radius of earth in NM (convert km to NM)
		double pi = 3.1415926535897932384636434;

		
		// Use these to scale x,y back to lat/long.  Assumes a rectangular grid centred on
		// the current position. 
		double distPerDegreeLong = 2 * pi * R * cos(y * pi / 180) / 360;
		double distPerDegreeLat = 2 * pi * R / 360;

		// Speeds in x & y (EW and NS) now in knots
		double vx = dxdt * distPerDegreeLong;
		double vy = dydt * distPerDegreeLat;

		// convert cartesian velocity to polar speed and heading
		last.speed = sqrt(vx * vx + vy * vy);
		last.heading = atan2(vy, vx) * 2 *pi; // in radians

		// TODO - convert 2nd derivatives into pitch and bank angle.
		// 2nd derivatives give acceleration but we need acceleration
		// normal to the velocity vector.  That normal accelaration
		// will then give us the bank angle assuming steady turn.
		last.pitch = 0;
		last.bank = 0;

		// Speed wrt world axes
		last.vwx = dxdt;
		last.vwy = dydt;
		last.vwz = dzdt;
	}

	return last;
}

