#pragma once
#include "stdafx.h"
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <ctype.h>
#include <time.h>
#include "Extension.h"
#include "TracePoint.h"

class IGCFile {

public:
	using TraceList = std::vector<TracePoint>;
	using ExtensionList = std::list<Extension>;

private:
	// final Map<String,String> flightInfo = new HashMap<String,String>();
	int flightIndex;		// of the ladder flight that produced this->
	std::string loggerId; 	//AFLARQK 
	time_t date;		 	//HFDTE300515
	int fixAccuracy; 	//HFFXA500
	std::string p1; 			//HFPLTPilotincharge:BRUCE PORTEOUS
	std::string p2; 			//HPCM2Crew2:
	std::string gliderType; 	//HFGTYGliderType:LS7
	std::string registration; //HFGIDGliderID:G-DFOG
	std::string datum;		//HFDTM100GPSDatum:WGS84
	std::string loggerFirmware; //HFRFWFirmwareVersion:Flarm-IGC06.01
	std::string loggerHardware; //HFRHWHardwareVersion:LXN-Flarm-IGC
	std::string loggerType;	//HFFTYFRType:LXN Red Box Flarm
	std::string gpsType; 	//HFGPSu-blox:TIM-LP,16,8191
	std::string pressureSensor; //HFPRSPressAltSensor:Intersema MS5534B,8191
	std::string competitionClass; //HFCCLCompetitionClass:Standard
	std::string competitionId; //HFCIDCompetitionID:952


	TraceList trace;
	ExtensionList extensions;

	void parseExtensionRecord(std::string line);
	void parseInfoRecord(std::string line);
	void parseTaskRecord(std::string line);

public:
	IGCFile();
	int getFlightIndex() const;
	void setFlightIndex(int index);
	std::string getLoggerId() const;
	void setLoggerId(std::string loggerId);
	time_t getDate() const;
	void setDate(time_t date);
	int getFixAccuracy() const;
	void setFixAccuracy(int fixAccuracy);
	std::string getP1() const;
	void setP1(std::string p1);
	std::string getP2() const;
	void setP2(std::string p2);
	std::string getGliderType() const;
	void setGliderType(std::string gliderType);
	std::string getRegistration() const;
	void setRegistration(std::string registration);
	std::string getDatum() const;
	void setDatum(std::string datum);
	std::string getLoggerFirmware() const;
	void setLoggerFirmware(std::string loggerFirmware);
	std::string getLoggerHardware() const;
	void setLoggerHardware(std::string loggerHardware);
	std::string getLoggerType()const;
	void setLoggerType(std::string loggerType);
	std::string getGpsType() const;
	void setGpsType(std::string gpsType);
	std::string getPressureSensor() const;
	void setPressureSensor(std::string pressureSensor);
	std::string getCompetitionClass() const;
	void setCompetitionClass(std::string competitionClass);
	std::string getCompetitionId() const;
	void setCompetitionId(std::string competitionId);

	TraceList& getTrace();
	void setTrace(const TraceList& trace);

	ExtensionList& getExtensions();

	void parse(const char* path);
	void parse(std::istream& is);

	void write(std::ostream& os);
};



