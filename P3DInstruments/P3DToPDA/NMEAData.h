#pragma once
#include <ostream>

class NMEAData
{
public:
	double latDegrees;		// North is positive, South negative
	double longDegrees;		// East is positive, West negative
	double gpsHeight;		// in metres
	double utcTime;			// in seconds from midnight;
	double groundSpeedKnots;
	double trackDegrees;
	double magneticVariationDegrees;
	double IASkph;
	double baroAltitudeMetres;
	double varioMetresSec;
	double headingDegrees;
	double windHeadingDegrees;
	double windSpeedKph;
	double year;
	double month;
	double day;

private:
	std::ostream& convertLat(std::ostream& os) const;
	std::ostream& convertLong(std::ostream& os) const;
	std::ostream& convertTime(std::ostream& os) const;

	void wrap(std::ostream& os, const std::string& in) const;

	unsigned int checksum(const std::string& in) const;


	void GPGGA(std::ostream& os);
	void GPRMC(std::ostream& os);
	void LXWP0(std::ostream& os);
public:

	void send(std::ostream& os);

	NMEAData(void);
	~NMEAData(void);
};

