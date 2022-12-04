#pragma once
#include <string>
#include <vector>
#include "CriticalSection.h"

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
	using Field = std::vector<std::string>;

	struct FieldDefinition {
		FieldType type;
		const char* name;
		const char* regex;
		bool multiple;
	};

	Field fields[FIELD_COUNT];
	static FieldDefinition definitions[FIELD_COUNT];

	CriticalSection guard;

	void skipSpaces(std::string::const_iterator& pos);
	std::string match(const char* pszRegex, std::string::const_iterator& pos, const std::string::const_iterator& end);
	bool parseSingleField(FieldType field, std::string::const_iterator& pos, const std::string::const_iterator& end);
	bool parseField(FieldType field, std::string::const_iterator& pos, const std::string::const_iterator& end);
	void clear();
	bool outputField(FieldType ft, std::string& output, bool needsSpace) const;
	void showField(FieldType field, std::string& text) const;

public:
	Metar();
	Metar(const std::string& metar);
	std::string text();
	std::string show();
	std::string parse(const std::string& metar);
	void merge(const Metar& metar);
	bool setField(FieldType field, const std::string& value); // true if successful set, false if invalid format
	std::string get(FieldType field) const;
	std::string getRepeated(FieldType field) const;
	static FieldType type(int idx);
	static bool multiple(FieldType field);
	static const char* typeName(FieldType field);
};

