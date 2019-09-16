#include "StdAfx.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "NMEAData.h"


	// Converts lat to appropriate degrees and N/S
	std::ostream& NMEAData::convertLat(std::ostream& os) const {
		
		int degrees, minutes, fraction;

		bool isNorth = latDegrees >= 0.0;
		double absLat = std::abs(latDegrees);

		// Lattitude, first 2 digits are whole degrees.
		degrees = (int)std::floor(absLat);
		absLat -= degrees;  // now fractional degrees.
		absLat *= 60;			// now minutes of arc.
		minutes = (int)std::floor(absLat);
		absLat -= minutes;		// decimal fraction of minute of arc.
		fraction = (int)std::floor(absLat * 10000);
		os << std::setw(2) << std::setfill('0') << degrees;
		os << std::setw(2) << std::setfill('0') << minutes;
		os << '.';
		os << std::setw(4) << std::setfill('0') << fraction;
		os << ','; 
		os << (isNorth ? 'N' : 'S');

		std::cout << "Lat: ";
		std::cout << std::setw(2) << std::setfill('0') << degrees;
		std::cout << ":" ; 
		std::cout << std::setw(2) << std::setfill('0') << minutes;
		std::cout << '.';
		std::cout << std::setw(4) << std::setfill('0') << fraction;
		std::cout << std::endl;

		return os;
	}

	// Converts long to appropriate degrees and E/W
	std::ostream& NMEAData::convertLong(std::ostream& os) const {

		int degrees, minutes, fraction;

		bool isEast = longDegrees >= 0.0;
		double absLong = std::abs(longDegrees);

		// Longditude, first 3 digits are whole degrees (+/- 180 degrees east or west).
		degrees = (int)std::floor(absLong);
		absLong -= degrees;  // now fractional degrees.
		absLong *= 60;			// now minutes of arc.
		minutes = (int)std::floor(absLong);
		absLong -= minutes;		// decimal fraction of minute of arc.
		fraction = (int)std::floor(absLong * 10000);
		os << std::setw(3) << std::setfill('0') << degrees;
		os << std::setw(2) << std::setfill('0') << minutes;
		os << '.';
		os << std::setw(4) << std::setfill('0') << fraction;
		os << ','; 
		os << (isEast ? 'E' : 'W');

		std::cout << "Long: ";
		std::cout << std::setw(3) << std::setfill('0') << degrees;
		std::cout << ":" ; 
		std::cout << std::setw(2) << std::setfill('0') << minutes;
		std::cout << '.';
		std::cout << std::setw(4) << std::setfill('0') << fraction;
		std::cout << std::endl;
		return os;
	}

	// Converts the UTC time to hhmmss.mmm
	std::ostream& NMEAData::convertTime(std::ostream& os) const {

		double utc = utcTime;

		int h = int(std::floor(utc / (60 * 60)));
		utc -= h * (60 * 60);  // take off whole hours

		int m = int(std::floor(utc / 60));
		utc -= m * 60;	// take off mins, just secs left.

		int s = int(std::floor(utc));
		utc -= s;

		int mS = int(std::floor(utc * 1000)); // milliseconds

		os << std::setw(2) << std::setfill('0') << h;
		os << std::setw(2) << std::setfill('0') << m;
		os << std::setw(2) << std::setfill('0') << s;
		os << '.' ;
		os << std::setw(3) << std::setfill('0') << mS;

		return os;
	}


	// Wraps the basic sentence by writing the leading $, *, checksum and CRLF
	void NMEAData::wrap(std::ostream& os, const std::string& in) const {
		os << "$";
		os << in;
		os << "*";
		os << std::hex << std::uppercase << std::setw(2) << std::setfill('0');
		os << checksum(in);
		os << std::dec << std::setw(0);
		os << "\r\n";
	}

	// Calculates the checksum for the core string.
	unsigned int NMEAData::checksum(const std::string& in) const {

		int len = (int)in.length();
		unsigned int cs = 0;
		for(int i=0; i<len; ++i) {
			unsigned char ch = in.at(i);
			cs ^= ch;
		}

		return cs;
	}

	// Writes the GPS GGA sentence to the output.
	void NMEAData::GPGGA(std::ostream& os){

		std::ostringstream oss;

		oss << "GPGGA,";

		convertTime(oss) << ",";
		convertLat(oss) << ",";
		convertLong(oss) << ",";
	
		oss << "1,";  // GPS fix
		oss << "12,"; // number of satellites
		oss << "10,"; // Dilution of Position (DoP)
		oss << std::setprecision(1) << std::fixed << gpsHeight << ',';
		oss << "M,"; // in Metres
		oss << ",,,,"; // height of geoid, units, DGPS time, DGPS station
		oss << "0000";

		wrap(os, oss.str());
	}



	void NMEAData::GPRMC(std::ostream& os){
		std::ostringstream oss;

		oss << "GPRMC,";
		convertTime(oss);
		oss << ",A,";  // valid

		convertLat(oss) << ",";

		convertLong(oss) << ",";

		oss << std::setw(0) << std::setprecision(2) << std::fixed;
		oss << groundSpeedKnots << ",";
		oss << trackDegrees << ",";

		oss << std::setw(2) << std::setfill('0');
		oss << int(day) << int(month) << int(year)%100 << ",";

		oss << std::setw(0) << std::fixed << std::setprecision(3) << std::abs(magneticVariationDegrees) << ",";
		oss << ((magneticVariationDegrees > 0) ? "E" : "W") << ",";  // 

		wrap(os, oss.str());
	}

	void NMEAData::LXWP0(std::ostream& os){
		std::ostringstream oss;

		oss << "LXWP0,";
		oss << "Y,";	// logging data
		oss << std::setw(0) << std::setprecision(2) << std::fixed;
		oss << IASkph << ",";
		oss << baroAltitudeMetres << ",";
		oss << varioMetresSec << ",";
		oss << ",,,,,"; // unknown fields 4 to 8
		oss << std::setprecision(1);
		oss << headingDegrees << ",";
		oss << windHeadingDegrees << ",";
		oss << windSpeedKph << ",";

		std::cout << std::setw(0) << std::setprecision(2) << std::fixed;
		std::cout << "IAS: " << IASkph << ", ";
		std::cout << "Baro: "<< baroAltitudeMetres << ", ";
		std::cout << "Vario: " << varioMetresSec << ", ";
		std::cout << ",,,,,"; // unknown fields
		std::cout << std::setprecision(1);
		std::cout << "Hdg: " << headingDegrees << ",";
		std::cout << "Wind: " << windHeadingDegrees << ", " << windSpeedKph;

		wrap(os, oss.str());
	}

	void NMEAData::send(std::ostream& os){
		GPGGA(os);
		GPRMC(os);
		LXWP0(os);
	}


NMEAData::NMEAData(void)
{
}


NMEAData::~NMEAData(void)
{
}
