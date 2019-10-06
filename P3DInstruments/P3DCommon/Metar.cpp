#include "stdafx.h"
#include <iostream>
#include "Metar.h"
#include <regex>

// Note that these must be in the correct order that corresponds to FieldType enumeration.
const char* Metar::regex[FIELD_COUNT] = {
	"^[A-Z]{4}(&A[0-9]{1,3})?", //	STATION,
	"^METAR|SPECI",				//	REPORT_TYPE,
	"^AUTO",					//  AUTO,
	"^COR",						//	COR,
	"^([0-9]{6}|[0-9]{4})Z?",	//	DATETIME,
	"^NIL",						//	NIL,
	"^([0-9]{3}|VRB)([0-9]{1,3}(G[0-9]{1,2})?(KMH|KT|MPS))(&D[0-9]{1,4}[NOLMHS][GMSI])?",	//	SURFACE_WIND,
	"^[0-9]{3}V[0-9]{3}",		//	MIN_MAX_WIND_DIR,
	"^(\\s*([0-9]{3}|VRB)([0-9]{1,3}(G[0-9]{1,2})?(KMH|KT|MPS))(&A[0-9]{1,4}[NOLMHS][GMSI])?)+",	//	WINDS_ALOFT,
	"^[0-9]{3}V[0-9]{3}",		//	MIN_MAX_WIND_DIR_ALOFT,
	"^CAVOK",					//	CAVOK,
	"^([M<][0-9]\\/[0-9]SM|[0-9]+SM|[0-9]\\/[0-9]SM|[0-9]+ [0-9]\\/[0-9]SM|[0-9]{1,3}KM|[0-9]+(NDV|M|NE|NW|SE|SW|N|S|E|W))(&B-?[0-9]{1,4}&D[0-9]{1,4})?",	//	VISIBILITY,
	"^R[0-9A-Z]{1,6}\\/([PM]?[0-9]{1,4}FT)|([0-9]{1,4}V[0-9]{1,4}FT)",	//	RVR,
	"^[+-VC]{1,3}(MI|PR|DC|DR|BL|SH|TS|FZ)(DZ|RA|SN|SG|IC|PE|GR|GS|UP|BR|FG|FU|VA|DU|SA|HZ|PY|PO|SQ|FC|SS|DS)", //	PRESENT_CONDITIONS,
	"^(FEW|SCT|BKN)(000|\\/\\/\\/)", //	PARTIAL_OBSCURATION,
	"^(\\s*(CLR|SKC|FEW|SCT|BKN|OVC|[1-8](CI|CS|CC|AS|AC|SC|NS|ST|CU|CB))[0-9]{1,3}(&(CI|CS|CC|AS|AC|SC|NS|ST|CU|CB)[0-9]{3}[FRA][NOLMHS][VLMHD][NRFHS][0-9]{1,3}[NTLMS])?)+",	//	SKY_CONDITIONS,
	"^(\\s*[0-9]{1,2}\\/[0-9]{1,2}(&A[0-9]{1,5})?)+", //	TEMPERATURE,
	"^[AQ][0-9]{1,5}",			//	ALTIMETER,
};

void Metar::skipSpaces(std::string::iterator& pos) {
	while (isspace(*pos)) {
		++pos;
	}
}

std::string Metar::match(const char* pszRegex, std::string::iterator& pos, const std::string::iterator& end)
{
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

void Metar::parseField(FieldType field, std::string::iterator& pos, const std::string::iterator& end)
{
	skipSpaces(pos);
	fields[field] = match(regex[field], pos, end);
}

void Metar::parse(const char* pszMetar)
{
	std::string metar(pszMetar);
	std::string::iterator pos = metar.begin();
	std::string::iterator end = metar.end();

	for (int t = STATION; t < FIELD_COUNT; ++t) {
		parseField((FieldType)t, pos, end);
	}
}

void Metar::merge(const Metar& metar)
{
	for (int t = STATION; t < FIELD_COUNT; ++t) {
		if (!metar.fields[t].empty()) {
			fields[t] = metar.fields[t];
		}
	}

	// Optional extension must not be set on update hence remove it.
	if (fields[STATION].length() > 4) {
		fields[STATION] = fields[STATION].substr(0, 4);
	}

}

// Sets a field value.  It either has to be a valid value or empty.
bool Metar::set(FieldType field, const std::string& value)
{
	bool valid = true;
	
	if (!value.empty()) {
		std::regex r(regex[field]);
		valid = (std::regex_match(value, r));
		if (!valid) {
			std::cout << "Invalid field " << field << " - " << value << " does not match " << regex[field] << std::endl;
		}
	}
	
	if(valid){
		fields[field] = value;
	}
	
	return valid;
}

std::string Metar::get(FieldType field) const
{
	return fields[field];
}

Metar::Metar()
{
}

Metar::Metar(const char* pszMetar)
{
	parse(pszMetar);
}

std::string Metar::text() const
{
	std::string text;
	bool needsSpace = false;
	for (int i = 0; i < FIELD_COUNT; ++i) {
		if (!fields[i].empty()) {
			if (needsSpace) {
				text.append(" ");
			}
			text.append(fields[i]);
			needsSpace = true;
		}
	}
	return text;
}

