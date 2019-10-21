#include "stdafx.h"
#include <time.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include "Folder.h"
#include "IGCFlightRecorder.h"
#include "..//P3DCommon/SimObjectDataRequest.h"


SimObjectData::DataItem IGCFlightRecorder::dataItems[] = {
	{"PLANE LATITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE","m",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS POSITION LAT","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS POSITION LON","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"GPS POSITION ALT","m",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU TIME","seconds",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU DAY OF MONTH","number",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU MONTH OF YEAR","number",SIMCONNECT_DATATYPE_FLOAT64},
	{"ZULU YEAR","number",SIMCONNECT_DATATYPE_FLOAT64},
	{0,0,SIMCONNECT_DATATYPE_INVALID}

};

IGCFlightRecorder::IGCFlightRecorder(Prepar3D* pTargetSim)
	: frData(this,pTargetSim,dataItems)
	, p3d(pTargetSim)
	, interval(4)
	, pRequest(0)
{
	frData.createDefinition();

	// Set up the hardware for the IGC file
	igc.setLoggerId("XPDAAA"); // non IGC approved device!
	igc.setGpsType("Prepar3d");
	igc.setDatum("WGS84");
	igc.setLoggerFirmware("P3DControl");
	igc.setLoggerHardware("Virtual");
	igc.setGpsType("Prepar3D");
	igc.setPressureSensor("Virtual");
}

IGCFlightRecorder::~IGCFlightRecorder()
{
	if (pRequest) {
		delete pRequest;
		pRequest = 0;
	}
	if (running()) {
		output.close();
	}
}

/*
A2.5.2 Long file name style.This uses a full set of characters in each field, a hyphen separating each field, the field
order being the same as in the short file name.For instance :
Short file name : 56HCXXX2.IGC
Long file name : 2015 - 06 - 17 - MMM - XXX - 02.IGC
MMM = manufacturer's three-letter IGC identifier (see table on next page)
XXX = unique FR Serial ID(S / ID); 3 alphanumeric characters
02 = second flight of the day
*/
File IGCFlightRecorder::createFileName(Directory& folder) {

	time_t now;
	time(&now);
	tm t;
	gmtime_s(&t, &now);

	int d = t.tm_mday;
	int m = t.tm_mon + 1;
	int y = t.tm_year + 1900;

	std::stringstream os;
	
	int idx = 0;

	File file("");
	do {
		++idx;
		os.clear();
		os.str("");

		os << std::setw(4) << std::setfill('0') << y
			<< "-"
			<< std::setw(2) << std::setfill('0') << m
			<< "-"
			<< std::setw(2) << std::setfill('0') << m
			<< "XPD-AAA"
			<< std::setw(2) << std::setfill('0') << idx
			<< ".IGC";

		file = folder.file(os.str());

	} while (file.exists());
	return file;
}

void IGCFlightRecorder::onData(Data* pData)
{
	assert(this);
	assert(pData);
	assert(running());
	// Use real rather than sim time.
	time_t now;
	time(&now);

	TracePoint p(now, pData->longitude, pData->latitude, (float)pData->gpsAltitude, (float)pData->altitude);
	p.write(output, igc.getExtensions());
}

void IGCFlightRecorder::start(Directory& directory)
{
	assert(!running());
	if (pRequest == 0) {
		pRequest = new SimObjectDataRequest(p3d, &frData, &p3d->userAircraft(), SIMCONNECT_PERIOD_SECOND,0,interval);
	}
	if (!running()) {
		File file = createFileName(directory);
		output.open(file);
		igc.write(output);
	}
}

void IGCFlightRecorder::stop()
{
	assert(running());
	if (pRequest) {
		delete pRequest;
		pRequest = 0;
	}
	if (running()) {
		output.close();
	}
}

bool IGCFlightRecorder::running()
{
	return output.is_open();
}

IGCFlightRecorder::FRData::FRData(IGCFlightRecorder* parent, Prepar3D* pTargetSim, SimObjectData::DataItem* pItems)
	: SimObjectData(pTargetSim, pItems)
	, parent(parent)
{
	createDefinition();
}

void IGCFlightRecorder::FRData::onData(void* pData, SimObject* pObject)
{
	parent->onData(reinterpret_cast<Data*>(pData));
}

IGCFlightRecorder::FRData::~FRData()
{
}
