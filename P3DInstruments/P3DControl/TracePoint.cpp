#include "stdafx.h"
#include <string>
#include <list>
#include <iostream>
#include <iomanip>
#include <exception>
#include <ctime>
#include <assert.h>
#include "Extension.h"
#include "TracePoint.h"

// Gets the time of the data point in format HHmmss
time_t TracePoint::readTime(const std::string& time) {
	int hh = stoi(time.substr(0, 2));
	int mm = stoi(time.substr(2, 2));
	int ss = stoi(time.substr(4, 2));

	return (((time_t)hh * 60) + mm) * 60 + ss;
}

TracePoint::TracePoint()
	: when(0)
	, eastings(0)
	, northings(0)
	, altGps(0)
	, altBaro(0)
	, fixAccuracy(0)
	, satellitesInUse(0)
	, engineNoiseLevel(0)
	, ias(0)
{
}

TracePoint::TracePoint(const std::string& bRecord, const std::list<Extension>& extensions)
	: TracePoint()
{

	try {
		// B 102027   5210970N  00006652W A 00066  00123  00107

		if (bRecord[0] != 'B') {
			throw std::exception("Not a B record");
		}

		std::string timeString = bRecord.substr(1, 6);
		when = readTime(timeString);

		std::string northingsText = bRecord.substr(7, 7);
		char NS = bRecord[14];
		int degreesNorth = stoi(northingsText.substr(0, 2)); // 00 -> 90
		double minutesNorth = stod(northingsText.substr(2)) / 1000.0;
		northings = degreesNorth + minutesNorth / 60.0;
		if (toupper(NS) == 'S') {
			northings = -northings;
		}

		std::string eastingsText = bRecord.substr(15, 8);
		char EW = bRecord[23];
		int degreesEast = stoi(eastingsText.substr(0, 3)); // 000 -> 180
		double minutesEast = stod(eastingsText.substr(3)) / 1000.0;
		eastings = degreesEast + minutesEast / 60.0;
		if (toupper(EW) == 'W') {
			eastings = -eastings;
		}

		char altFlag = bRecord[24];


		std::string altBaroText = bRecord.substr(25, 5);
		altBaro = stof(altBaroText);

		if (altFlag == 'A' && bRecord.length() >= 35) { // not all files have barometric altitude
			std::string altGpsText = bRecord.substr(30, 5);
			altGps = stof(altGpsText);
		}
		else {
			altGps = 0;
		}

		for (Extension e : extensions) {
			std::string text = bRecord.substr(e.getStart() - 1, e.length());
			if (e.getTypeCode() == "FXA") { // fix accuracy
				fixAccuracy = stoi(text);
			}
			else if (e.getTypeCode() == "SIU") { // satellites in use
				satellitesInUse = stoi(text);
			}
			else if (e.getTypeCode() == "ENL") { // Engine Noise Level
				engineNoiseLevel = stoi(text);
			}
			else if (e.getTypeCode() == "IAS") {  // Indicated Airspeed
				ias = stoi(text);
			}
		}

	}
	catch (std::exception& e) {
		std::cout << "Problem parsing B record (" << e.what() << "): " << bRecord << std::endl;
		throw e;
	}

}



/**
 * @param when
 * @param eastings
 * @param northings
 * @param altGps
 * @param altBaro
 */
TracePoint::TracePoint(time_t when, double eastings, double northings,
	float altGps, float altBaro)
	: when(when)
	, eastings(eastings)
	, northings(northings)
	, altGps(altGps)
	, altBaro(altBaro)
	, fixAccuracy(0)
	, satellitesInUse(99)
	, engineNoiseLevel(0)
	, ias(0)
{
}

time_t TracePoint::getWhen() const {
	return when;
}

double TracePoint::getEastings() const {
	return eastings;
}

double TracePoint::getNorthings() const {
	return northings;
}

float TracePoint::getAltGps() const {
	return altGps;
}

float TracePoint::getAltBaro() const {
	return altBaro;
}

int TracePoint::getFixAccuracy() const {
	return fixAccuracy;
}

int TracePoint::getSatellitesInUse() const {
	return satellitesInUse;
}

int TracePoint::getEngineNoiseLevel() const {
	return engineNoiseLevel;
}

int TracePoint::getIas() const {
	return ias;
}

void TracePoint::write(std::ostream& os, const std::list<Extension>& extensions) const
{
	os << "B";

	// Time
	tm t;
	gmtime_s(&t , &when );
	os << std::setw(2) << std::setfill('0') << t.tm_hour;
	os << std::setw(2) << std::setfill('0') << t.tm_min;
	os << std::setw(2) << std::setfill('0') << t.tm_sec;

	// Northings
	char NS = (northings > 0) ? 'N' : 'S';
	double lat = northings;
	if (lat < 0) lat = -lat;
	int degrees = (int)floor(lat);
	lat -= degrees;
	lat *= 60;  // now in minutes.
	int minutes = (int)floor(lat * 1000);
	
	os << std::setw(2) << std::setfill('0') << degrees;
	os << std::setw(5) << std::setfill('0') << minutes;
	os << NS;

	// Eastings
	char EW = (eastings > 0) ? 'E' : 'W';
	double lng = eastings;
	if (lng < 0) lng = -lng;
	degrees = (int)floor(lng);
	lng -= degrees;
	lng *= 60; // to minutes
	minutes = (int)floor(lng * 1000);

	os << std::setw(3) << std::setfill('0') << degrees;
	os << std::setw(5) << std::setfill('0') << minutes;
	os << EW;

	// Altitude
	os << 'A';
	os << std::setw(5) << std::setfill('0') << (int)altBaro;
	os << std::setw(5) << std::setfill('0') << (int)altGps;


#ifndef NDEBUG
	int pos = 0;
#endif

	for (auto it = extensions.begin(); it != extensions.end(); ++it) {
#ifndef NDEBUG
		// Implicit assumption that the extensions are in order by position on the line
		// Check it:
		assert(pos < it->getStart());
		pos = it->getFinish();
#endif
		if (it->getTypeCode() == "FXA") { // fix accuracy
			os << std::setw(it->length()) << std::setfill('0') << fixAccuracy;
		} else if (it->getTypeCode() == "SIU") { // Satellites in use
			os << std::setw(it->length()) << std::setfill('0') << satellitesInUse;
		} else if (it->getTypeCode() == "ENL") { // Environmental Noise Level
			os << std::setw(it->length()) << std::setfill('0') << engineNoiseLevel;
		} else if(it->getTypeCode() == "MOP") { // Method of propulsion
			os << std::setw(it->length()) << std::setfill('0') << 0;
		}
	}

	os << std::endl;
}

