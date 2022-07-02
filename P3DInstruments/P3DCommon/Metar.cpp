#include "stdafx.h"
#include <iostream>
#include <assert.h>
#include "Metar.h"
#include <regex>

// These define the format of an allowable Metar in P3D
// Note that these must be in the correct order that corresponds to FieldType enumeration.
// Fields are type (for checking order at runtime)
// name of field for error reporting
// Regex to match a single entry
// flag to say if multiple entries are used. 
Metar::FieldDefinition Metar::definitions[FIELD_COUNT] = {
	{ STATION,			"STATION",			"^[A-Z0-9]{4}(&A[0-9]{1,3})?", false },
	{ REPORT_TYPE,		"REPORT_TYPE",		"^METAR|SPECI", false},
	{ AUTO,				"AUTO",				"^AUTO", false },
	{ COR,				"COR",				"^COR", false },
	{ DATETIME,			"DATETIME",			"^([0-9]{6}|[0-9]{4})Z?", false },
	{ NIL,				"NIL",				"^NIL",	false },
	{ SURFACE_WIND,		"SURFACE_WIND",		"^([0-9]{3}|VRB)([0-9]{1,3}(G[0-9]{1,2})?(KMH|KT|MPS))(&D[0-9]{1,4}[NOLMHS][GMSI])?", false },
	{ MIN_MAX_WIND_DIR,	"MIN_MAX_WIND_DIR", "^[0-9]{3}V[0-9]{3}", false },
	{ WINDS_ALOFT,		"WINDS_ALOFT",		"^([0-9]{3}|VRB)([0-9]{1,3}(G[0-9]{1,2})?(KMH|KT|MPS))(&A[0-9]{1,4}[NOLMHS][GMSI])?", true },
	{ MIN_MAX_WIND_DIR_ALOFT, "MIN_MAX_WIND_DIR_ALOFT", "^[0-9]{3}V[0-9]{3}", true },
	{ CAVOK,			"CAVOK",			"^CAVOK", false},
	{ VISIBILITY,		"VISIBILITY",		"^([M<][0-9]\\/[0-9]SM|[0-9]+SM|[0-9]\\/[0-9]SM|[0-9]+ [0-9]\\/[0-9]SM|[0-9]{1,3}KM|[0-9]+(NDV|M|NE|NW|SE|SW|N|S|E|W))(&B-?[0-9]{1,4}&D[0-9]{1,4})?", true},
	{ RVR,				"RVR",				"^R[0-9A-Z]{1,6}\\/([PM]?[0-9]{1,4}FT)|([0-9]{1,4}V[0-9]{1,4}FT)",	true},
	{ PRESENT_CONDITIONS,"PRESENT_CONDITIONS", "^[+-VC]{1,3}(MI|PR|DC|DR|BL|SH|TS|FZ)(DZ|RA|SN|SG|IC|PE|GR|GS|UP|BR|FG|FU|VA|DU|SA|HZ|PY|PO|SQ|FC|SS|DS)", true},
	{ PARTIAL_OBSCURATION,"PARTIAL_OBSCURATION", "^(FEW|SCT|BKN)(000|\\/\\/\\/)", false},
	{ SKY_CONDITIONS,	"SKY_CONDITIONS",	"^(CLR|SKC)|(FEW|SCT|BKN|OVC|[1-8](CI|CS|CC|AS|AC|SC|NS|ST|CU|CB))[0-9]{1,3}(&(CI|CS|CC|AS|AC|SC|NS|ST|CU|CB)[0-9]{3}[FRA][NOLMHS][VLMHD][NRFHS][0-9]{1,3}[NTLMS])?", true},
	{ TEMPERATURE,		"TEMPERATURE",		"^[0-9]{1,2}\\/[0-9]{1,2}(&A[0-9]{1,5})?", true },
	{ ALTIMETER,		"ALTIMETER",		"^[AQ][0-9]{1,5}", false}
};

// Skips spaces in the input string moving the pos iterator on as necessary.
void Metar::skipSpaces(std::string::const_iterator& pos) {
	assert(this);
	while (isspace(*pos)) {
		++pos;
	}
}

// Tries to match a field on the input starting at the pos iterator.  
// Returns either the matched string or an empty field, if matched
// the pos iterator is moved forward to skip the matched string.
std::string Metar::match(const char* pszRegex, std::string::const_iterator& pos, const std::string::const_iterator& end)
{
	assert(this);
	assert(pszRegex);

	std::string value;
	std::regex r(pszRegex);
	std::sregex_iterator next(pos, end, r);
	std::sregex_iterator endOfSequence;
	if (next != endOfSequence) {
		std::smatch match = *next;
		value = match.str();
		pos += match.length();
	}

	return value;
}

// Parses a single field value and adds it if present.
// Returns true if there was input that matches the given field definition.
bool Metar::parseSingleField(FieldType field, std::string::const_iterator& pos, const std::string::const_iterator& end)
{
	assert(this);
	assert(field >= STATION && field < FIELD_COUNT);
	assert(definitions[field].type == field); // sanity check to make sure all in the right order

	// Maybe nothing
	if (pos == end) {
		return false;
	}

	skipSpaces(pos);

	// In case of trailing spaces...
	if (pos == end) {
		return false;
	}

	std::string value = match(definitions[field].regex, pos, end);
	if (!value.empty()) {
		fields[field].push_back(value);
	}
	return !value.empty();
}

// Parses zero or more instances of a given field.
bool Metar::parseField(FieldType field, std::string::const_iterator& pos, const std::string::const_iterator& end)
{
	assert(this);
	assert(field >= STATION && field < FIELD_COUNT);
	bool hasContent = parseSingleField(field, pos, end);

	if (hasContent && definitions[field].multiple) {
		while (parseSingleField(field, pos, end)) { 
			; // do nothing
		}
	}
	return hasContent;
}

// Clears the metar so it contains no data.
void Metar::clear()
{
	assert(this);
	for (int t = STATION; t < FIELD_COUNT; ++t) {
		fields[t].clear();
	}
}

std::string Metar::parse(const std::string& metar)
{
	assert(this);

	CriticalSection::Lock lock(guard);

	clear();

	std::string::const_iterator pos = metar.begin();
	std::string::const_iterator end = metar.end();

	parseField(STATION, pos, end);
	parseField(REPORT_TYPE, pos, end);
	parseField(AUTO, pos, end);
	parseField(COR, pos, end);
	parseField(DATETIME, pos, end);
	parseField(NIL, pos, end);
	parseField(SURFACE_WIND, pos, end);
	parseField(MIN_MAX_WIND_DIR, pos, end);

	// Winds aloft can have optional min/max wind dir with each
	while (parseSingleField(WINDS_ALOFT, pos, end)) {
		if (!parseSingleField(MIN_MAX_WIND_DIR_ALOFT, pos, end)) {
			fields[MIN_MAX_WIND_DIR_ALOFT].push_back("");
		}
	}
		
	parseField(CAVOK, pos, end);
	parseField(VISIBILITY, pos, end);
	parseField(RVR, pos, end);
	parseField(PRESENT_CONDITIONS, pos, end);
	parseField(PARTIAL_OBSCURATION, pos, end);
	parseField(SKY_CONDITIONS, pos, end);
	parseField(TEMPERATURE, pos, end);
	parseField(ALTIMETER, pos, end);

	std::string residual(pos, end);
	while (!residual.empty() && isspace(residual.back())) {
		residual.pop_back();
	}

	// Optional extension must not be set on update hence remove it.
	if (!fields[STATION].empty() && fields[STATION][0].length() > 4) {
		fields[STATION][0] = fields[STATION][0].substr(0, 4);
	}

	return residual;
}

void Metar::merge(const Metar& metar)
{
	CriticalSection::Lock lock(guard);

	for (int t = STATION; t < FIELD_COUNT; ++t) {
		if (!metar.fields[t].empty()) {
			fields[t] = metar.fields[t];
		}
	}

	// Optional extension must not be set on update hence remove it.
	if (fields[STATION][0].length() > 4) {
		fields[STATION][0] = fields[STATION][0].substr(0, 4);
	}

}

// Sets a field value.  It either has to be a valid value or empty.
bool Metar::setField(FieldType field, const std::string& value)
{
	assert(this);
	assert(field >= STATION && field < FIELD_COUNT);

	CriticalSection::Lock lock(guard);

	fields[field].clear();

	std::string::const_iterator pos = value.begin();
	std::string::const_iterator end = value.end();

	bool valid = parseField(field, pos, end);
	return valid;
}


std::string Metar::get(FieldType field) const
{
	assert(this);
	assert(field >= STATION && field < FIELD_COUNT);

	std::string value;
	if (!fields[field].empty()) {
		value = fields[field][0];
	}
	return value;
}

std::string Metar::getRepeated(FieldType field) const
{
	assert(this);
	assert(field >= STATION && field < FIELD_COUNT);

	std::string text;
	bool needsSpace = false;

	for (int i = 0; i < fields[field].size(); ++i) {
		if (needsSpace) {
			text.append(" ");
		}
		text.append(fields[field][i]);
		needsSpace = true;
	}
	return text;
}

Metar::FieldType Metar::type(int idx)
{
	assert(idx >= STATION && idx < FIELD_COUNT);
	return definitions[idx].type;
}

bool Metar::multiple(FieldType field) 
{
	assert(field >= STATION && field < FIELD_COUNT);
	return definitions[field].multiple;
}

const char* Metar::typeName(FieldType field) 
{
	assert(field >= STATION && field < FIELD_COUNT);
	return definitions[field].name;
}

Metar::Metar()
{
}

Metar::Metar(const std::string& metar)
{
	assert(this);
	parse(metar);
}

bool Metar::outputField(FieldType ft, std::string& output, bool needsSpace) const {
	assert(this);
	assert(ft >= STATION && ft < FIELD_COUNT);

	for (Field::const_iterator iter = fields[ft].begin();
		iter != fields[ft].end();
		++iter) {
		if (needsSpace) {
			output.append(" ");
		}
		output.append(*iter);
		needsSpace = true;
	}
	return needsSpace;
}

// Regenerates the metar as a text string.
std::string Metar::text() 
{
	assert(this);

	CriticalSection::Lock lock(guard);

	std::string text;
	bool needsSpace = false;
	needsSpace = outputField(STATION, text, needsSpace);
	needsSpace = outputField(REPORT_TYPE, text, needsSpace);
	needsSpace = outputField(AUTO, text, needsSpace);
	needsSpace = outputField(COR, text, needsSpace);
	needsSpace = outputField(DATETIME, text, needsSpace);
	needsSpace = outputField(NIL, text, needsSpace);
	needsSpace = outputField(SURFACE_WIND, text, needsSpace);
	needsSpace = outputField(MIN_MAX_WIND_DIR, text, needsSpace);

	for(int i=0; i<fields[WINDS_ALOFT].size(); ++i){
		if (needsSpace) {
			text.append(" ");
		}
		text.append(fields[WINDS_ALOFT][i]);
		needsSpace = true;

		std::string mmwda = fields[MIN_MAX_WIND_DIR_ALOFT][i];
		if (!mmwda.empty()) {
			text.append(" ");
			text.append(mmwda);
		}
	}

	needsSpace = outputField(CAVOK, text, needsSpace);
	needsSpace = outputField(VISIBILITY, text, needsSpace);
	needsSpace = outputField(RVR, text, needsSpace);
	needsSpace = outputField(PRESENT_CONDITIONS, text, needsSpace);
	needsSpace = outputField(PARTIAL_OBSCURATION, text, needsSpace);
	needsSpace = outputField(SKY_CONDITIONS, text, needsSpace);
	needsSpace = outputField(TEMPERATURE, text, needsSpace);
	needsSpace = outputField(ALTIMETER, text, needsSpace);

	return text;
}

void Metar::showField(FieldType field, std::string& text) const {
	if (!fields[field].empty()) {
		if (!text.empty()) {
			text.append(" | ");
		}

		if (fields[field].size() > 0) {
			text.append(definitions[field].name);
			text.append(" : ");
			text.append(fields[field][0]);
		}
		for (int i = 1; i < fields[field].size(); ++i) {
			text.append(" , ");
			text.append(fields[field][i]);
		}
	}
	
}

std::string Metar::show()
{
	assert(this);

	std::string text;

	CriticalSection::Lock lock(guard);

	showField(STATION, text);
	showField(REPORT_TYPE, text);
	showField(AUTO, text);
	showField(COR, text);
	showField(DATETIME, text);
	showField(NIL, text);
	showField(SURFACE_WIND, text);
	showField(MIN_MAX_WIND_DIR, text);
	showField(WINDS_ALOFT, text);
	showField(MIN_MAX_WIND_DIR_ALOFT, text);
	showField(CAVOK, text);
	showField(VISIBILITY, text);
	showField(RVR, text);
	showField(PRESENT_CONDITIONS, text);
	showField(PARTIAL_OBSCURATION, text);
	showField(SKY_CONDITIONS, text);
	showField(TEMPERATURE, text);
	showField(ALTIMETER, text);

	return text;
}

