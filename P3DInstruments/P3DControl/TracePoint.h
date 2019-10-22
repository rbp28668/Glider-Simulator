#pragma once
#include "stdafx.h"
#include <string>
#include <list>
#include <iostream>
#include <exception>
#include <ctime>
#include "Extension.h"

// A single trace point (B Record) in an IGC file.
class TracePoint {

	time_t when;		// time
	double eastings;   // decimal degrees, west is +ve
	double northings;  // decimal degrees, north is +ve
	float altGps;	   // meters
	float altBaro;     // meters

	int fixAccuracy;
	int satellitesInUse;
	int engineNoiseLevel;
	int ias;

	time_t readTime(const std::string& time);

public:

	TracePoint();
	TracePoint(const std::string& bRecord, const std::list<Extension>& extensions);
	TracePoint(time_t when, double eastings, double northings, float altGps, float altBaro);
	time_t getWhen() const;
	double getEastings() const;
	double getNorthings() const;
	float getAltGps() const;
	float getAltBaro() const;
	int getFixAccuracy() const;
	int getSatellitesInUse() const;
	int getEngineNoiseLevel() const;
	int getIas() const;

	void write(std::ostream& os, const std::list<Extension>& extensions) const;
};

