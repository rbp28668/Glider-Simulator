#include "stdafx.h"
#include "Failures.h"

SimObjectData::DataItem Failures::dataItems[] = {
	{"PARTIAL PANEL AIRSPEED","enum",SIMCONNECT_DATATYPE_INT32},
	{"PARTIAL PANEL ALTIMETER","enum",SIMCONNECT_DATATYPE_INT32},
	{"PARTIAL PANEL ELECTRICAL","enum",SIMCONNECT_DATATYPE_INT32},
	{"PARTIAL PANEL PITOT","enum",SIMCONNECT_DATATYPE_INT32},
	{"PARTIAL PANEL TURN COORDINATOR","enum",SIMCONNECT_DATATYPE_INT32},
};

SimObjectData::DataItem* Failures::items()
{
	return dataItems;
}

int Failures::itemCount()
{
	return sizeof(dataItems) / sizeof(dataItems[0]);
}

void Failures::onData(void* pvData)
{
	struct Data* pData = reinterpret_cast<Data*>(pvData);
	CriticalSection::Lock lock(csData);
	data = *pData;
}

Failures::Failures(Prepar3D* pSim)
	: SimObjectData(pSim)
{
	createDefinition();
}

Failures::~Failures()
{
}

Failures::Data Failures::current()
{
	CriticalSection::Lock lock(csData);
	Data d(data);
	return d;
}

void Failures::failAirspeed(FAILURE_MODE mode)
{
	Data d;
	{
		CriticalSection::Lock lock(csData);
		data.airspeed = mode;
		d = data;
	}
	send(&d, sizeof(d));
}

void Failures::failAltimeter(FAILURE_MODE mode)
{
	Data d;
	{
		CriticalSection::Lock lock(csData);
		data.altimeter = mode;
		d = data;
	}
	send(&d, sizeof(d));
}

void Failures::failElectrical(FAILURE_MODE mode)
{
	Data d;
	{
		CriticalSection::Lock lock(csData);
		data.electrical = mode;
		d = data;
	}
	send(&d, sizeof(d));
}

void Failures::failPitot(FAILURE_MODE mode)
{
	Data d;
	{
		CriticalSection::Lock lock(csData);
		data.pitot = mode;
		d = data;
	}
	send(&d, sizeof(d));
}

void Failures::failTurnCoordinator(FAILURE_MODE mode)
{
	Data d;
	{
		CriticalSection::Lock lock(csData);
		data.turn_coordinator = mode;
		d = data;
	}
	send(&d, sizeof(d));
}

void Failures::clearAll()
{
	Data d;
	{
		CriticalSection::Lock lock(csData);
		data.airspeed = OK;
		data.altimeter = OK;
		data.electrical = OK;
		data.pitot = OK;
		data.turn_coordinator = OK;
		d = data;
	}
	send(&d, sizeof(d));

}
