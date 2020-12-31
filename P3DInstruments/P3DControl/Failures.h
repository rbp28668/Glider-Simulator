#pragma once
#include "../P3DCommon/SimObjectData.h"
#include "../P3DCommon/CriticalSection.h"

class Failures : public SimObjectData {
public:
	enum FAILURE_MODE {
		OK = 0,
		FAIL = 1,
		BLANK = 2
	} ;

	struct Data {
		int airspeed;
		int altimeter;
		int electrical;
		int pitot;
		int turn_coordinator;
		Data() : airspeed(OK), altimeter(OK), electrical(OK), pitot(OK), turn_coordinator(OK) {}
		Data(const Data& other) 
			: airspeed(other.airspeed)
			, altimeter(other.altimeter)
			, electrical(other.electrical)
			, pitot(other.pitot)
			, turn_coordinator(other.turn_coordinator) {}
	};

private:
	static DataItem dataItems[];
	Data data;
	CriticalSection csData;

public:

	virtual DataItem* items();
	virtual int itemCount();

	virtual void onData(void* pData);

	Failures(Prepar3D* pSim);
	~Failures();

	Data current();
	void failAirspeed(FAILURE_MODE mode = FAIL);
	void failAltimeter(FAILURE_MODE mode = FAIL);
	void failElectrical(FAILURE_MODE mode = FAIL);
	void failPitot(FAILURE_MODE mode = FAIL);
	void failTurnCoordinator(FAILURE_MODE mode = FAIL);
	void clearAll();
};

