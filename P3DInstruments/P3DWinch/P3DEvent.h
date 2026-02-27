#pragma once
#pragma once
#include <map>
#include "../P3DCommon/Prepar3D.h"

// Call basic P3D commands via client events.
class P3DEvent
{
public:
	typedef enum EVENT_ID {
		START_VALUE = Prepar3D::EVENT_ID::LAST_P3D_EVENT, // Note that without this we get duplicate event ID exeptions. 

		FREEZE_LATITUDE_LONGITUDE_SET, // 	Freezes the lat/lon position of the aircraft.
		FREEZE_ALTITUDE_SET, // Freezes the altitude of the aircraft..
		FREEZE_ATTITUDE_SET, // Freezes the attitude (pitch, bank and heading) of the aircraft.
		POSITION_FREEZE_USER, // Toggles the position freeze (Lat/Lon, Altitude and Attitude) of the controlled SimObject.
		POSITION_FREEZE_ALL, // Toggles the position freeze(Lat / Lon, Altitudeand Attitude) of all SimObjects.

	} EventID;

private:
	Prepar3D* p3d;
	void setupEvents();
	bool mapEvent(EventID event, const char* name);
	typedef std::map<std::string, EventID> EventLookup;
	EventLookup eventLookup;

public:

	P3DEvent(Prepar3D* p3d);
	~P3DEvent();
	bool dispatchEvent(EventID event, DWORD  dwData, DWORD dwId = SIMCONNECT_OBJECT_ID_USER);

};


