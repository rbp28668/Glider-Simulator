#pragma once
#include <string>

// See SimConnect_WeatherSetObservation for details of metar format.
// http://www.prepar3d.com/SDKv4/sdk/simconnect_api/references/world_functions.html#Metar%20Data%20Format
// GLOB&A0 000000Z 00000KT&D985NG 27020KT&A2026NG 27025KT&A6026NG 100KM&B-423&D3048 2CU058&CU001FNMN000N 6CI394&CI001FNMN000N 15/05 Q1013 @@@ 66 15 270 20 | 198 15 270 25 |
class Metar
{
public:
	enum FieldType {
		STATION,
		REPORT_TYPE,
		AUTO,
		COR,
		DATETIME,
		NIL,
		SURFACE_WIND,
		MIN_MAX_WIND_DIR,
		WINDS_ALOFT,
		MIN_MAX_WIND_DIR_ALOFT,
		CAVOK,
		VISIBILITY,
		RVR,
		PRESENT_CONDITIONS,
		PARTIAL_OBSCURATION,
		SKY_CONDITIONS,
		TEMPERATURE,
		ALTIMETER,
		FIELD_COUNT
	};

private:
	std::string fields[FIELD_COUNT];
	static const char* regex[FIELD_COUNT];

	void skipSpaces(std::string::iterator& pos);
	std::string match(const char* pszRegex, std::string::iterator& pos, const std::string::iterator& end);
	void parseField(FieldType field, std::string::iterator& pos, const std::string::iterator& end);
	

public:
	Metar();
	Metar(const char* pszMetar);
	std::string text() const;
	void parse(const char* pszMetar);
	void merge(const Metar& metar);
	bool set(FieldType field, const std::string& value); // true if successful set, false if invalid format
	std::string get(FieldType field) const;
};

