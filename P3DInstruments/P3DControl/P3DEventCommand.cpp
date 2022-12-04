#include "stdafx.h"
#include <assert.h>
#include <string>
#include "P3DEventCommand.h"


// See http://www.prepar3d.com/SDKv4/sdk/references/variables/event_ids.html for list of event IDs

P3DEventCommand::P3DEventCommand(Prepar3D * p3d)
{
	assert(p3d != 0);
	this->p3d = p3d;
	setupEvents();
}

P3DEventCommand::~P3DEventCommand()
{
}

void P3DEventCommand::setupEvents() {
	mapEvent(THROTTLE_FULL,"THROTTLE_FULL");	
	mapEvent(THROTTLE_INCR,"THROTTLE_INCR");	
	mapEvent(THROTTLE_INCR_SMALL,"THROTTLE_INCR_SMALL");
	mapEvent(THROTTLE_DECR,"THROTTLE_DECR");
	mapEvent(THROTTLE_DECR_SMALL,"THROTTLE_DECR_SMALL");
	mapEvent(THROTTLE_CUT,"THROTTLE_CUT");
	mapEvent(INCREASE_THROTTLE,"INCREASE_THROTTLE");
	mapEvent(DECREASE_THROTTLE,"DECREASE_THROTTLE");
	mapEvent(THROTTLE_SET,"THROTTLE_SET");
	mapEvent(AXIS_THROTTLE_SET, "AXIS_THROTTLE_SET");
	mapEvent(AXIS_MIXTURE_SET, "AXIS_MIXTURE_SET");
	mapEvent(AXIS_PROPELLER_SET, "AXIS_PROPELLER_SET");
	mapEvent(MIXTURE_SET, "MIXTURE_SET");
	mapEvent(MIXTURE_RICH, "MIXTURE_RICH");
	mapEvent(MIXTURE_INCR, "MIXTURE_INCR");
	mapEvent(MIXTURE_INCR_SMALL, "MIXTURE_INCR_SMALL");
	mapEvent(MIXTURE_DECR, "MIXTURE_DECR");
	mapEvent(MIXTURE_DECR_SMALL, "MIXTURE_DECR_SMALL");
	mapEvent(MIXTURE_LEAN, "MIXTURE_LEAN");
	mapEvent(MIXTURE_SET_BEST, "MIXTURE_SET_BEST");
	mapEvent(PROP_PITCH_SET, "PROP_PITCH_SET");
	mapEvent(PROP_PITCH_LO, "PROP_PITCH_LO");
	mapEvent(PROP_PITCH_INCR, "PROP_PITCH_INCR");
	mapEvent(PROP_PITCH_INCR_SMALL, "PROP_PITCH_INCR_SMALL");
	mapEvent(PROP_PITCH_DECR, "PROP_PITCH_DECR");
	mapEvent(PROP_PITCH_DECR_SMALL, "PROP_PITCH_DECR_SMALL");
	mapEvent(PROP_PITCH_HI, "PROP_PITCH_HI");
	mapEvent(MAGNETO_OFF, "MAGNETO_OFF");
	mapEvent(MAGNETO_RIGHT, "MAGNETO_RIGHT");
	mapEvent(MAGNETO_LEFT, "MAGNETO_LEFT");
	mapEvent(MAGNETO_BOTH, "MAGNETO_BOTH");
	mapEvent(MAGNETO_START, "MAGNETO_START");
	mapEvent(ENGINE_AUTO_START, "ENGINE_AUTO_START");
	mapEvent(ENGINE_AUTO_SHUTDOWN, "ENGINE_AUTO_SHUTDOWN");
	mapEvent(ENGINE, "ENGINE");
	mapEvent(FLAPS_UP, "FLAPS_UP");
	mapEvent(FLAPS_1, "FLAPS_1");
	mapEvent(FLAPS_2, "FLAPS_2");
	mapEvent(FLAPS_3, "FLAPS_3");
	mapEvent(FLAPS_DOWN, "FLAPS_DOWN");
	mapEvent(ELEV_TRIM_DN, "ELEV_TRIM_DN");
	mapEvent(ELEV_TRIM_UP, "ELEV_TRIM_UP");
	mapEvent(SPOILERS_ON, "SPOILERS_ON");
	mapEvent(SPOILERS_OFF, "SPOILERS_OFF");
	mapEvent(ELEVATOR_SET, "ELEVATOR_SET");			// Sets elevator position (-16383 - +16383)
	mapEvent(AILERON_SET, "AILERON_SET");			// Sets aileron position (-16383 - +16383)
	mapEvent(RUDDER_SET, "RUDDER_SET");				// Sets rudder position (-16383 - +16383)
	mapEvent(BAROMETRIC, "BAROMETRIC");
	mapEvent(KOHLSMAN_INC, "KOHLSMAN_INC");
	mapEvent(KOHLSMAN_DEC, "KOHLSMAN_DEC");
	mapEvent(KOHLSMAN_SET, "KOHLSMAN_SET");
	mapEvent(RESET_G_FORCE_INDICATOR, "RESET_G_FORCE_INDICATOR");
	mapEvent(TOGGLE_VACUUM_FAILURE, "TOGGLE_VACUUM_FAILURE");			// Toggle vacuum system failure	Shared Cockpit
	mapEvent(TOGGLE_ELECTRICAL_FAILURE, "TOGGLE_ELECTRICAL_FAILURE");		// Toggle electrical system failure	Shared Cockpit
	mapEvent(TOGGLE_PITOT_BLOCKAGE, "TOGGLE_PITOT_BLOCKAGE");			// Toggles blocked pitot tube	Shared Cockpit
	mapEvent(TOGGLE_STATIC_PORT_BLOCKAGE, "TOGGLE_STATIC_PORT_BLOCKAGE");		// Toggles blocked static port	Shared Cockpit
	mapEvent(TOGGLE_HYDRAULIC_FAILURE, "TOGGLE_HYDRAULIC_FAILURE");			// Toggles hydraulic system failure	Shared Cockpit
	mapEvent(TOGGLE_TOTAL_BRAKE_FAILURE, "TOGGLE_TOTAL_BRAKE_FAILURE");		// Toggles brake failure(both)	Shared Cockpit
	mapEvent(TOGGLE_ENGINE1_FAILURE, "TOGGLE_ENGINE1_FAILURE");
	mapEvent(GEAR_UP, "GEAR_UP");
	mapEvent(GEAR_DOWN, "GEAR_DOWN");
	mapEvent(TOW_PLANE_RELEASE, "TOW_PLANE_RELEASE");
	mapEvent(TOW_PLANE_REQUEST, "TOW_PLANE_REQUEST");
	mapEvent(TOGGLE_WATER_BALLAST_VALVE, "TOGGLE_WATER_BALLAST_VALVE");
	mapEvent(TOGGLE_VARIOMETER_SWITCH, "TOGGLE_VARIOMETER_SWITCH");
	mapEvent(TOGGLE_TURN_INDICATOR_SWITCH, "TOGGLE_TURN_INDICATOR_SWITCH");
	mapEvent(PAUSE_TOGGLE, "PAUSE_TOGGLE");
	mapEvent(PAUSE_ON, "PAUSE_ON");
	mapEvent(PAUSE_OFF, "PAUSE_OFF");
	mapEvent(SELECT_1, "SELECT_1");
	mapEvent(SELECT_2, "SELECT_2");
	mapEvent(SELECT_3, "SELECT_3");
	mapEvent(SELECT_4, "SELECT_4");
	mapEvent(MINUS, "MINUS");
	mapEvent(PLUS, "PLUS");
	mapEvent(SITUATION_SAVE, "SITUATION_SAVE");
	mapEvent(SITUATION_RESET, "SITUATION_RESET");
	mapEvent(TOGGLE_AIRCRAFT_LABELS, "TOGGLE_AIRCRAFT_LABELS");
	mapEvent(EXIT, "EXIT");
	mapEvent(ABORT, "ABORT");
	mapEvent(SIM_RESET, "SIM_RESET");
	mapEvent(REFRESH_SCENERY, "REFRESH_SCENERY");
	mapEvent(CAPTURE_SCREENSHOT, "CAPTURE_SCREENSHOT");
	mapEvent(AUTORUDDER_TOGGLE, "AUTORUDDER_TOGGLE");
	mapEvent(FREEZE_LATITUDE_LONGITUDE_TOGGLE, "FREEZE_LATITUDE_LONGITUDE_TOGGLE"); //Turns the freezing of the lat/lon position of the aircraft (either user or AI controlled) on or off. 
	mapEvent(FREEZE_LATITUDE_LONGITUDE_SET, "FREEZE_LATITUDE_LONGITUDE_SET");	// 	Freezes the lat/lon position of the aircraft.
	mapEvent(FREEZE_ALTITUDE_TOGGLE, "FREEZE_ALTITUDE_TOGGLE");					//	Turns the freezing of the altitude of the aircraft on or off.
	mapEvent(FREEZE_ALTITUDE_SET, "FREEZE_ALTITUDE_SET");						// Freezes the altitude of the aircraft..
	mapEvent(FREEZE_ATTITUDE_TOGGLE, "FREEZE_ATTITUDE_TOGGLE");					// Turns the freezing of the attitude (pitch, bank and heading) of the aircraft on or off.
	mapEvent(FREEZE_ATTITUDE_SET, "FREEZE_ATTITUDE_SET");						// Freezes the attitude (pitch, bank and heading) of the aircraft.
	mapEvent(POSITION_FREEZE_USER, "POSITION_FREEZE_USER");						// Toggles the position freeze (Lat/Lon, Altitude and Attitude) of the controlled SimObject.
	mapEvent(POSITION_FREEZE_ALL, "POSITION_FREEZE_ALL");						// Toggles the position freeze(Lat / Lon, Altitudeand Attitude) of all SimObjects.
	mapEvent(SLEW_TOGGLE, "SLEW_TOGGLE");										// Toggles slew on / off	Shared Cockpit(Pilot only)
	mapEvent(SLEW_OFF, "SLEW_OFF"); // 	Turns slew off	Shared Cockpit(Pilot only)
	mapEvent(SLEW_ON, "SLEW_ON"); // 	Turns slew on	Shared Cockpit(Pilot only)
	mapEvent(SLEW_SET, "SLEW_SET"); // 	Sets slew on / off(1, 0)	Shared Cockpit(Pilot only)
	mapEvent(SLEW_RESET, "SLEW_RESET"); // 	Stop slew and reset pitch, bank, and heading all to zero.Shared Cockpit(Pilot only)
	mapEvent(SLEW_ALTIT_UP_FAST, "SLEW_ALTIT_UP_FAST"); // 	Slew upward fast	Shared Cockpit(Pilot only)
	mapEvent(SLEW_ALTIT_UP_SLOW, "SLEW_ALTIT_UP_SLOW"); // 	Slew upward slow	Shared Cockpit(Pilot only)
	mapEvent(SLEW_ALTIT_FREEZE, "SLEW_ALTIT_FREEZE"); // 	Stop vertical slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_ALTIT_DN_SLOW, "SLEW_ALTIT_DN_SLOW"); // 	Slew downward slow	Shared Cockpit(Pilot only)
	mapEvent(SLEW_ALTIT_DN_FAST, "SLEW_ALTIT_DN_FAST"); // 	Slew downward fast	Shared Cockpit(Pilot only)
	mapEvent(SLEW_ALTIT_PLUS, "SLEW_ALTIT_PLUS"); // 	Increase upward slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_ALTIT_MINUS, "SLEW_ALTIT_MINUS"); // 	Decrease upward slew 	Shared Cockpit(Pilot only)
	mapEvent(SLEW_PITCH_DN_FAST, "SLEW_PITCH_DN_FAST"); // 	Slew pitch downward fast	Shared Cockpit(Pilot only)
	mapEvent(SLEW_PITCH_DN_SLOW, "SLEW_PITCH_DN_SLOW"); // 	Slew pitch downward slow	Shared Cockpit(Pilot only)
	mapEvent(SLEW_PITCH_FREEZE, "SLEW_PITCH_FREEZE"); // 	Stop pitch slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_PITCH_UP_SLOW, "SLEW_PITCH_UP_SLOW"); // 	Slew pitch up slow	Shared Cockpit(Pilot only)
	mapEvent(SLEW_PITCH_UP_FAST, "SLEW_PITCH_UP_FAST"); // 	Slew pitch upward fast	Shared Cockpit(Pilot only)
	mapEvent(SLEW_PITCH_PLUS, "SLEW_PITCH_PLUS"); // 	Increase pitch up slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_PITCH_MINUS, "SLEW_PITCH_MINUS"); // 	Decrease pitch up slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_BANK_MINUS, "SLEW_BANK_MINUS"); // 	Increase left bank slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_AHEAD_PLUS, "SLEW_AHEAD_PLUS"); // 	Increase forward slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_BANK_PLUS, "SLEW_BANK_PLUS"); // 	crease right bank slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_LEFT, "SLEW_LEFT"); // 	Slew to the left	Shared Cockpit(Pilot only)
	mapEvent(SLEW_FREEZE, "SLEW_FREEZE"); // 	Stop all slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_RIGHT, "SLEW_RIGHT"); // 	Slew to the right	Shared Cockpit(Pilot only)
	mapEvent(SLEW_HEADING_MINUS, "SLEW_HEADING_MINUS"); // 	Increase slew heading to the left	Shared Cockpit(Pilot only)
	mapEvent(SLEW_AHEAD_MINUS, "SLEW_AHEAD_MINUS"); // 	Decrease forward slew	Shared Cockpit(Pilot only)
	mapEvent(SLEW_HEADING_PLUS, "SLEW_HEADING_PLUS"); // 	Increase slew heading to the right	Shared Cockpit(Pilot only)

}

bool P3DEventCommand::mapEvent(EventID event, const char* name) {
	HRESULT hr = ::SimConnect_MapClientEventToSimEvent(p3d->getHandle(), event, name);
	eventLookup[name] = event;
	return hr != S_OK;
}

bool P3DEventCommand::dispatchEvent(EventID event, DWORD  dwData, DWORD dwId)
{
	// SimConnect_MapClientEventToSimEvent for your key event definitions(along with the extra functions involved in this process) and use SimConnect_TransmitClientEvent to trigger that event.
	// See http://www.prepar3d.com/SDKv4/sdk/references/variables/event_ids.html for event IDs

	HRESULT hr = ::SimConnect_TransmitClientEvent(p3d->getHandle(), dwId, event, dwData, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	return hr != S_OK;
}

bool P3DEventCommand::dispatchEvent(const std::string & eventName, DWORD  dwData, DWORD dwId)
{
	EventLookup::iterator pos = eventLookup.find(eventName);
	if (pos != eventLookup.end()) {
		return dispatchEvent(pos->second, dwData, dwId);
	}
	return true;
}

