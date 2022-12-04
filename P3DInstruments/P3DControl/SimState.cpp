#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <sstream>
#include "SimState.h"
#include "Simulator.h"

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
// Note this should match the Data type based initposition above.
SimObjectData::DataItem SimState::dataItems[] = {
	{"PLANE LATITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE LONGITUDE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE ALTITUDE","feet",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE PITCH DEGREES","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE BANK DEGREES","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"PLANE HEADING DEGREES TRUE","degrees",SIMCONNECT_DATATYPE_FLOAT64},
	{"SIM ON GROUND","bool",SIMCONNECT_DATATYPE_INT32},
    {"AIRSPEED TRUE","knots",SIMCONNECT_DATATYPE_INT32},
	{"TOW CONNECTION","bool",SIMCONNECT_DATATYPE_INT32},

};

SimObjectData::DataItem* SimState::items() {
	return dataItems;
}

int SimState::itemCount() {
	return sizeof(dataItems) / sizeof(dataItems[0]);
}

void SimState::onData(void* pvData, SimObject* pObject) {
	assert(this);
	assert(pvData);
	assert(pObject);

	struct Data* pData = reinterpret_cast<Data*>(pvData);
	CriticalSection::Lock lock(csData);
	if (pData) {
		data = *pData;

		if (!getSim()->isPaused()) {
			buffer.add(this->data);
		}

		if (tugConnected && (data.onTow == 0)) {
			pSim->tugReleased();
		}

		tugConnected = (data.onTow != 0);
	}
}

SimState::SimState(Prepar3D* pSim)
	: pSim(static_cast<Simulator*>(pSim))
	, SimObjectData(pSim)
	, init(pSim)
	, buffer(600)  // 10 mins
	, tugConnected(false)
{
	assert(pSim);
	createDefinition();
	init.createDefinition();
}


SimState::~SimState(void)
{
}

SimState::Data SimState::current()
{
	assert(this);
	CriticalSection::Lock lock(csData);
	Data d(data);
	return d;
}

void SimState::update(const Data& data, FIELDS fields)
{
	assert(this);

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

void SimState::set(const Data& data)
{ 
	assert(this);

	SIMCONNECT_DATA_INITPOSITION pos;
	pos.Airspeed = data.Airspeed;
	pos.Altitude = data.Altitude;
	pos.Bank = data.Bank;
	pos.Heading = data.Heading;
	pos.Latitude = data.Latitude;
	pos.Longitude = data.Longitude;
	pos.OnGround = data.OnGround;
	pos.Pitch = data.Pitch;

	init.send(&pos, SIMCONNECT_OBJECT_ID_USER);
}

int SimState::historyLength()
{
	assert(this);

	CriticalSection::Lock lock(csData);
	int result = (int)buffer.count;
	return result;
}

SimState::Data SimState::history(int n)
{
	assert(this);
	assert(n >= 0);

	CriticalSection::Lock lock(csData);
	Data d = buffer.back(n);  // copy operation in lock scope
	return d;
}

SimState::Data SimState::rewindTo(int n)
{
	assert(this);
	assert(n >= 0);

	CriticalSection::Lock lock(csData);
	Data d = buffer.rewind(n);  // copy operation in lock scope
	return d;
}

void SimState::clear()
{
	buffer.reset();
}

SimState::Data::Data()
	: onTow(0)
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

	if (n > count) n = (int)count; // can't go back more than we have data.

	// Go back N items
	int pos = (int)head - n;
	if (pos < 0) pos += (int)len;
	
	return buffer[pos];
}

SimState::Data& SimState::Buffer::rewind(int n)
{
	CriticalSection::Lock lock(cs);

	if (n > count) n = (int)count; // can't go back more than we have data.

	// Go back N items
	int pos = (int)head - n;
	if (pos < 0) pos += (int)len;

	// Reset buffer to where we go back to.
	head = ((size_t)pos + 1) % len;
	count -= n;

	return buffer[pos];
}

// Resets the buffer effectively making it empty.
void SimState::Buffer::reset()
{
	CriticalSection::Lock lock(cs);
	head = count = 0;
}

