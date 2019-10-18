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
	size_t count = trace.count();
	xx = new double[count];
	yy = new double[count];
	zz = new double[count];
	tt = new double[count];

	time_t tStart = trace.valueAt(0).when;

	for (int i = 0; i < count; ++i) {
		Point p = trace.valueAt(i);
		xx[i] = p.x; // long
		yy[i] = p.y; // lat
		zz[i] = p.z; // altitude feet
		tt[i] = (double)(p.when - tStart);
	}
	maxTime = (double) (trace.valueAt(count - 1).when - tStart);

	xSpline = new Spline<double>(tt, xx, count);
	ySpline = new Spline<double>(tt, yy, count);
	zSpline = new Spline<double>(tt, zz, count);

}

Position Trace::positionAtTime(double t)
{
	if (t < maxTime) {
		// First and second derivatives
		double dxdt, dydt, dzdt;
		double d2xdt2, d2ydt2, d2zdt2;

		// Points at this time.
		double x = xSpline->point(t, dxdt, d2xdt2);
		double y = ySpline->point(t, dydt, d2ydt2);
		double z = zSpline->point(t, dzdt, d2zdt2);

		last.lon = x;
		last.lat = y;
		last.alt = z;

		double R = 6371 / 1.852; // radius of earth in NM (convert km to NM)
		double pi = 3.1415926535897932384636434;

		
		// Use these to scale x,y back to lat/long.  Assumes a rectangular grid centred on
		// the current position. 
		double distPerDegreeLong = 2 * pi * R * cos(y * pi / 180) / 360;
		double distPerDegreeLat = 2 * pi * R / 360;

		// Speeds in x & y (EW and NS) now in knots.  Note that dxdt and dydt are speeds
		// in degrees per second.  Hence 3600 scale factor to convert seconds to hours.
		double vx = dxdt * distPerDegreeLong * 3600;
		double vy = dydt * distPerDegreeLat * 3600;

		// convert cartesian velocity to polar speed and heading
		// Note that with heading, atan2 returns zero along the x axis
		// that increases anti-clockwise whereas heading starts at north
		// and increases clockwise.
		last.speed = sqrt(vx * vx + vy * vy);
		last.heading = pi/2 - atan2(vy, vx); // in radians
		if (last.heading < 0) {
			last.heading += 2 * pi;
		}

		// Convert 2nd derivatives into pitch and bank angle.
		// 2nd derivatives give acceleration but we need acceleration
		// normal to the velocity vector.  That normal accelaration
		// will then give us the bank angle assuming steady turn.

		if (last.speed >= 10) { // don't do pitch and bank below 10 kts
			
			// Convert velocity into unit vector (vux,vuy).  This gives the angle
			// we want to rotate the acceleration through to figure out what's the
			// normal acceleration (i.e. determines the angle of bank).
			double vux = vx / last.speed;
			double vuy = vy / last.speed;

			// Accelerations are given in degrees lat/long per second per second.
			// Convert to metres per second per second (via NM!)
			double ax = d2xdt2 * distPerDegreeLong * 1852; // now in metres.
			double ay = d2ydt2 * distPerDegreeLat * 1852;

			// this should give the normal (at right angles to) acceleration
			// with +ve left.  Note as vux and vuy are equivalent to cos and -sin
			// for a normal rotation matrix.
			double ayr = vux * ay - vuy * ax;  // rotated and hence normal acceleration.

			// Note that ayr should be in metres per sec per sec.
			last.bank = atan2(ayr,9.81); // 
		}
		else { // assume level on ground
			last.pitch = 0;
			last.bank = 0;
		}

		// Speed wrt world axes
		last.vwx = dxdt;
		last.vwy = dydt;
		last.vwz = dzdt;
	}

	return last;
}

