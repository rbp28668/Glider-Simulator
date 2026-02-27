#include "P3DEvent.h"

#include "stdafx.h"
#include <assert.h>
#include <string>
#include "P3DEvent.h"


// See http://www.prepar3d.com/SDKv5/sdk/references/variables/event_ids.html for list of event IDs

P3DEvent::P3DEvent(Prepar3D* p3d)
{
	assert(p3d != 0);
	this->p3d = p3d;
	setupEvents();
}

P3DEvent::~P3DEvent()
{
}

void P3DEvent::setupEvents() {
	mapEvent(FREEZE_LATITUDE_LONGITUDE_SET, "FREEZE_LATITUDE_LONGITUDE_SET");	// 	Freezes the lat/lon position of the aircraft.
	mapEvent(FREEZE_ALTITUDE_SET, "FREEZE_ALTITUDE_SET");						// Freezes the altitude of the aircraft..
	mapEvent(FREEZE_ATTITUDE_SET, "FREEZE_ATTITUDE_SET");						// Freezes the attitude (pitch, bank and heading) of the aircraft.
	mapEvent(POSITION_FREEZE_USER, "POSITION_FREEZE_USER");						// Toggles the position freeze (Lat/Lon, Altitude and Attitude) of the controlled SimObject.
	mapEvent(POSITION_FREEZE_ALL, "POSITION_FREEZE_ALL");						// Toggles the position freeze(Lat / Lon, Altitudeand Attitude) of all SimObjects.
}

bool P3DEvent::mapEvent(EventID event, const char* name) {
	HRESULT hr = ::SimConnect_MapClientEventToSimEvent(p3d->getHandle(), event, name);
	eventLookup[name] = event;
	return hr != S_OK;
}

bool P3DEvent::dispatchEvent(EventID event, DWORD  dwData, DWORD dwId)
{
	// SimConnect_MapClientEventToSimEvent for your key event definitions(along with the extra functions involved in this process) and use SimConnect_TransmitClientEvent to trigger that event.
	// See http://www.prepar3d.com/SDKv4/sdk/references/variables/event_ids.html for event IDs

	HRESULT hr = ::SimConnect_TransmitClientEvent(p3d->getHandle(), dwId, event, dwData, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	return hr != S_OK;
}


