#pragma once
#include "../P3DCommon/SimObjectData.h"
#include "../P3DCommon/CriticalSection.h"
#include "InitPosition.h"

class Prepar3D;
class Simulator;

class SimState : public SimObjectData
{
public:
	
	struct Data : public SIMCONNECT_DATA_INITPOSITION {
		int32_t onTow;  // 1 when attached to tug, 0 otherwise.
		Data();
	};

	enum FIELDS {
		LATLONG = 1,
		ALTITUDE = 2,
		PITCH_ROLL = 4,
		HEADING = 8,
		GROUND = 16,
		AIRSPEED = 32,
		ALL = 0xFFFF
	};

private:
	Simulator* pSim;
	static DataItem dataItems[];
	Data data;
	CriticalSection csData;
	InitPosition init;
	bool tugConnected;

	struct Buffer {
		Data* buffer;
		size_t len;
		size_t head; // where next item will go
		size_t count;
		CriticalSection cs;

		Buffer(int size);
		~Buffer();
		void add(const Data& data);
		Data& back(int n);
		Data& rewind(int n);
		void reset();
	};

	Buffer buffer;

public:
	virtual DataItem* items();
	virtual int itemCount();

	virtual void onData(void* pData, SimObject* pObject);

	SimState(Prepar3D* pSim);
	~SimState();

	Data current(); // current position.
	void update(const Data& data, FIELDS fields = FIELDS::ALL);
	void set(const Data& data); // Uses initposition to set all fields.
	int historyLength();
	Data history(int n);
	Data rewindTo(int n);
	void clear();
};

