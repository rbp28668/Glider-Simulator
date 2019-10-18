#include "stdafx.h"
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctype.h>
#include "IGCFile.h"


static std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first)
	{
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}


IGCFile::IGCFile() {
}



int IGCFile::getFlightIndex() const {
	return flightIndex;
}

void IGCFile::setFlightIndex(int index) {
	this->flightIndex = index;
}

std::string IGCFile::getLoggerId() const {
	return loggerId;
}

void IGCFile::setLoggerId(std::string loggerId) {
	this->loggerId = loggerId;
}

time_t IGCFile::getDate() const {
	return date;
}

void IGCFile::setDate(time_t date) {
	this->date = date;
}

int IGCFile::getFixAccuracy() const {
	return fixAccuracy;
}

void IGCFile::setFixAccuracy(int fixAccuracy) {
	this->fixAccuracy = fixAccuracy;
}


std::string IGCFile::getP1() const {
	return p1;
}

void IGCFile::setP1(std::string p1) {
	this->p1 = p1;
}

std::string IGCFile::getP2() const {
	return p2;
}

void IGCFile::setP2(std::string p2) {
	this->p2 = p2;
}

std::string IGCFile::getGliderType() const {
	return gliderType;
}

void IGCFile::setGliderType(std::string gliderType) {
	this->gliderType = gliderType;
}

std::string IGCFile::getRegistration() const {
	return registration;
}

void IGCFile::setRegistration(std::string registration) {
	this->registration = registration;
}

std::string IGCFile::getDatum() const {
	return datum;
}


void IGCFile::setDatum(std::string datum) {
	this->datum = datum;
}

std::string IGCFile::getLoggerFirmware() const {
	return loggerFirmware;
}


void IGCFile::setLoggerFirmware(std::string loggerFirmware) {
	this->loggerFirmware = loggerFirmware;
}


/**
 * @return the loggerHardware
 */
std::string IGCFile::getLoggerHardware() const {
	return loggerHardware;
}


/**
 * @param loggerHardware the loggerHardware to set
 */
void IGCFile::setLoggerHardware(std::string loggerHardware) {
	this->loggerHardware = loggerHardware;
}


/**
 * @return the loggerType
 */
std::string IGCFile::getLoggerType()const {
	return loggerType;
}


/**
 * @param loggerType the loggerType to set
 */
void IGCFile::setLoggerType(std::string loggerType) {
	this->loggerType = loggerType;
}


/**
 * @return the gpsType
 */
std::string IGCFile::getGpsType() const {
	return gpsType;
}


/**
 * @param gpsType the gpsType to set
 */
void IGCFile::setGpsType(std::string gpsType) {
	this->gpsType = gpsType;
}


/**
 * @return the pressureSensor
 */
std::string IGCFile::getPressureSensor() const {
	return pressureSensor;
}


/**
 * @param pressureSensor the pressureSensor to set
 */
void IGCFile::setPressureSensor(std::string pressureSensor) {
	this->pressureSensor = pressureSensor;
}


/**
 * @return the competitionClass
 */
std::string IGCFile::getCompetitionClass() const {
	return competitionClass;
}


/**
 * @param competitionClass the competitionClass to set
 */
void IGCFile::setCompetitionClass(std::string competitionClass) {
	this->competitionClass = competitionClass;
}


/**
 * @return the competitionId
 */
std::string IGCFile::getCompetitionId() const {
	return competitionId;
}


/**
 * @param competitionId the competitionId to set
 */
void IGCFile::setCompetitionId(std::string competitionId) {
	this->competitionId = competitionId;
}


/**
 * @return the trace
 */
IGCFile::TraceList& IGCFile::getTrace() {
	return trace;
}


/**
 * @param trace the trace to set
 */
void IGCFile::setTrace(const TraceList& trace) {
	this->trace = trace;
}


void IGCFile::parse(const char* path) {
	std::ifstream ifs(path);
	try {
		parse(ifs);
	}
	catch (std::exception& e) {
		ifs.close();
		throw e;
	}
}

void IGCFile::parse(std::istream& is) {

	std::string line;
	while (!is.eof()) {
		std::getline(is, line);

		if (std::all_of(line.begin(), line.end(), isspace)) {
			continue;
		}
		try {

			switch (line.at(0)) {
			case 'B':
				trace.push_back(TracePoint(line, extensions));
				break;

			case 'H':
				parseInfoRecord(line);
				break;

			case 'C':
				parseTaskRecord(line);
				break;

			case 'I':
				parseExtensionRecord(line);
				break;

			case 'A':
				loggerId = line.substr(1);
				break;
			}

		}
		catch (std::exception& e) {
			std::cout << "Invalid line ignored " << e.what() << std::endl;
			std::cout << line << std::endl;
		}
	}

}


/**
 * Parses an I or extension record. This defines what extensions
 * are included in a B or fix record.
 * @param line
 */
void IGCFile::parseExtensionRecord(std::string line) {
	line = trim(line);
	int count = stoi(line.substr(1, 2));
	int idx = 3;  // point at first char after extension count;
	for (int i = 0; i < count; ++i) {
		int start = stoi(line.substr(idx, 2));
		int finish = stoi(line.substr(idx + 2, 2));
		std::string typeCode = line.substr(idx + 4, 3);

		Extension e(typeCode, start, finish);
		extensions.push_back(e);

		idx += 7;
	}

}



void IGCFile::parseInfoRecord(std::string line) {

	line = trim(line);


	if (line.find("HFDTE") == 0) { //HFDTE300515
		//SimpleDateFormat sdf = new SimpleDateFormat("ddMMyy");
		//date = sdf.parse(line.substring(5));
		std::string ddmmyy = line.substr(5);
		struct tm timeinfo;
		timeinfo.tm_hour = 0;
		timeinfo.tm_min = 0;
		timeinfo.tm_sec = 0;
		timeinfo.tm_isdst = 0;
		timeinfo.tm_mday = stoi(ddmmyy.substr(0, 2));
		timeinfo.tm_mon = stoi(ddmmyy.substr(2, 2)) - 1; // range 0..11
		timeinfo.tm_year = stoi(ddmmyy.substr(4, 2));
		// Y2K style bodge.  
		if (timeinfo.tm_year < 70) {
			timeinfo.tm_year += 100;
		}
		date = mktime(&timeinfo);
	}
	else if (line.find("HFFXA") == 0) {
		fixAccuracy = stoi(line.substr(5));
	}
	else {
		size_t idx = line.find_first_of(':');
		std::string tag = line.substr(0, 5);
		if (idx != std::string::npos) {
			std::string value = line.substr(idx + 1);
			if (tag == "HFPLT") p1 = value;
			else if (tag == "HPCM2") p2 = value;
			else if (tag == "HFGTY") gliderType = value;
			else if (tag == "HFGID") registration = value;
			else if (tag == "HFDTM") datum = value;
			else if (tag == "HFRFW") loggerFirmware = value;
			else if (tag == "HFRHW") loggerHardware = value;
			else if (tag == "HFFTY") loggerType = value;
			else if (tag == "HFGPS") gpsType = value;
			else if (tag == "HFPRS") pressureSensor = value;
			else if (tag == "HFCCL") competitionClass = value;
			else if (tag == "HFCID") competitionId = value;
		}

	}

}

void IGCFile::parseTaskRecord(std::string line) {
	// TODO Auto-generated method stub
}


