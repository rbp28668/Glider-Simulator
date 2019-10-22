#include "stdafx.h"
#include "SimState.h"
#include <iostream>
#include <sstream>

/* For reference see https://www.prepar3d.com/SDKv4/sdk/simconnect_api/references/structures_and_enumerations.html#SIMCONNECT_DATA_INITPOSITION
struct SIMCONNECT_DATA_INITPOSITION {
	double  Latitude;
	double  Longitude;
	double  Altitude;
	double  Pitch;
	double  Bank;
	double  Heading;
	DWORD  OnGround;
	DWORD  Airspeed;
};
*/
// Note this should match the structure above.
SimObjectData::DataItem SimState::dataItems[] = {
	{"PLANE LATITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE","feet",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE PITCH DEGREES","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE BANK DEGREES","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE HEADING DEGREES TRUE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"SIM ON GROUND","bool",SIMCONNECT_DATATYPE_INT32},
    {"AIRSPEED TRUE","knots",SIMCONNECT_DATATYPE_INT32},
};

SimObjectData::DataItem* SimState::items() {
	return dataItems;
}

int SimState::itemCount() {
	return sizeof(dataItems) / sizeof(dataItems[0]);
}

void SimState::onData(void* pvData, SimObject* pObject) {
	struct Data* pData = reinterpret_cast<Data*>(pvData);
	CriticalSection::Lock lock(csData);
	data = *pData;

	if (!getSim()->isPaused()) {
		buffer.add(this->data);
	}
}

SimState::SimState(Prepar3D* pSim)
	: SimObjectData(pSim)
	, buffer(600)  // 10 mins
{
	createDefinition();
}


SimState::~SimState(void)
{
}

SimState::Data SimState::current()
{
	CriticalSection::Lock lock(csData);
	Data d(data);
	return d;
}

void SimState::update(const Data& data, FIELDS fields)
{
	
	if (fields & FIELDS::LATLONG) {
		this->data.Latitude = data.Latitude;			
		this->data.Longitude = data.Longitude;		
	}
	if (fields & FIELDS::ALTITUDE) {
		this->data.Altitude = data.Altitude;			
	}
	if (fields & FIELDS::PITCH_ROLL) {
		this->data.Pitch = data.Pitch;			
		this->data.Bank = data.Bank;				
	}
	if (fields & FIELDS::HEADING) {
		this->data.Heading = data.Heading;	//{"PLANE HEADING DEGREES TRUE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	}
	if (fields & FIELDS::GROUND) {
		this->data.OnGround = data.OnGround;
	}
	if (fields & FIELDS::AIRSPEED) {
		this->data.Airspeed = data.Airspeed;
	}

	send(&this->data, sizeof(this->data));

}

int SimState::historyLength()
{
	CriticalSection::Lock lock(csData);
	int result = buffer.count;
	return result;
}

SimState::Data SimState::history(int n)
{
	CriticalSection::Lock lock(csData);
	Data d = buffer.back(n);  // copy operation in lock scope
	return d;
}

SimState::Data::Data()
{
	Latitude = 0;
	Longitude = 0;
	Altitude = 0;
	Pitch = 0;
	Bank = 0;
	OnGround = 0;
	Airspeed = 0;
}

SimState::Buffer::Buffer(int size)
	: buffer(0)
	, len(size)
	, head(0)
	, count(0)
{
	buffer = new SimState::Data[size];
}

SimState::Buffer::~Buffer()
{
	delete[] buffer;
}

void SimState::Buffer::add(const Data& data)
{
	CriticalSection::Lock lock(cs);
	buffer[head] = data;
	head = (head + 1) % len;
	++count;
	if (count > len) count = len;
}

SimState::Data& SimState::Buffer::back(int n)
{
	CriticalSection::Lock lock(cs);

	if (n > count) n = count; // can't go back more than we have data.

	// Go back N items
	int pos = head - n;
	if (pos < 0) pos += len;
	
	// Reset buffer to where we go back to.
	head = (pos + 1) % len;
	count -= n;

	return buffer[pos];
}
