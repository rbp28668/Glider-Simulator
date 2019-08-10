#pragma once
#include <map>
#include "../P3DCommon/Prepar3D.h"
class P3DEventCommand
{
public:
	typedef enum EVENT_ID {
		FLAPS_UP,
		FLAPS_1,
		FLAPS_2,
		FLAPS_3,
		FLAPS_4,
		FLAPS_DOWN,
		ELEV_TRIM_DN,
		ELEV_TRIM_UP,
		SPOILERS_ON,
		SPOILERS_OFF,
		BAROMETRIC,
		KOHLSMAN_INC,
		KOHLSMAN_DEC,
		KOHLSMAN_SET,
		RESET_G_FORCE_INDICATOR,
		GEAR_UP,
		GEAR_DOWN,
		TOW_PLANE_RELEASE,
		TOW_PLANE_REQUEST,
		TOGGLE_WATER_BALLAST_VALVE,
		TOGGLE_VARIOMETER_SWITCH,
		TOGGLE_TURN_INDICATOR_SWITCH,
		PAUSE_TOGGLE,
		PAUSE_ON,
		PAUSE_OFF,
		SITUATION_SAVE,
		SITUATION_RESET,
		EXIT,
		ABORT,
		SIM_RESET,
		REFRESH_SCENERY,
		CAPTURE_SCREENSHOT,
		SLEW_TOGGLE,	// Toggles slew on / off	Shared Cockpit(Pilot only)
		SLEW_OF,		// Turns slew off	Shared Cockpit(Pilot only)
		SLEW_ON,		// Turns slew on	Shared Cockpit(Pilot only)
		SLEW_SET,		// 	Sets slew on / off(1, 0)	Shared Cockpit(Pilot only)
		SLEW_RESET,		// 	Stop slew and reset pitch, bank, and heading all to zero.Shared Cockpit(Pilot only)
		SLEW_ALTIT_UP_FAST, // 	Slew upward fast	Shared Cockpit(Pilot only)
		SLEW_ALTIT_UP_SLOW, // 	Slew upward slow	Shared Cockpit(Pilot only)
		SLEW_ALTIT_FREEZE,  // 	Stop vertical slew	Shared Cockpit(Pilot only)
		SLEW_ALTIT_DN_SLOW, // 	Slew downward slow	Shared Cockpit(Pilot only)
		SLEW_ALTIT_DN_FAST,  // 	Slew downward fast	Shared Cockpit(Pilot only)
		SLEW_ALTIT_PLUS,	// 	Increase upward slew	Shared Cockpit(Pilot only)
		SLEW_ALTIT_MINUS,	// 	Decrease upward slew 	Shared Cockpit(Pilot only)
		SLEW_PITCH_DN_FAST,  // 	Slew pitch downward fast	Shared Cockpit(Pilot only)
		SLEW_PITCH_DN_SLOW,  // 	Slew pitch downward slow	Shared Cockpit(Pilot only)
		SLEW_PITCH_FREEZE,  // 	Stop pitch slew	Shared Cockpit(Pilot only)
		SLEW_PITCH_UP_SLOW,  // 	Slew pitch up slow	Shared Cockpit(Pilot only)
		SLEW_PITCH_UP_FAST,  // 	Slew pitch upward fast	Shared Cockpit(Pilot only)
		SLEW_PITCH_PLUS,	// 	Increase pitch up slew	Shared Cockpit(Pilot only)
		SLEW_PITCH_MINUS,	// 	Decrease pitch up slew	Shared Cockpit(Pilot only)
		SLEW_BANK_MINUS,	// 	Increase left bank slew	Shared Cockpit(Pilot only)
		SLEW_AHEAD_PLUS,	// 	Increase forward slew	Shared Cockpit(Pilot only)
		SLEW_BANK_PLUS,		// 	crease right bank slew	Shared Cockpit(Pilot only)
		SLEW_LEFT,			// 	Slew to the left	Shared Cockpit(Pilot only)
		SLEW_FREEZE,		// 	Stop all slew	Shared Cockpit(Pilot only)
		SLEW_RIGHT,			// 	Slew to the right	Shared Cockpit(Pilot only)
		SLEW_HEADING_MINUS,  // 	Increase slew heading to the left	Shared Cockpit(Pilot only)
		SLEW_AHEAD_MINUS,	// 	Decrease forward slew	Shared Cockpit(Pilot only)
		SLEW_HEADING_PLUS,  // 	Increase slew heading to the right	Shared Cockpit(Pilot only)

	} EventID;

private:
	Prepar3D* p3d;
	void setupEvents();
	bool mapEvent(EventID event, const char* name);
	typedef std::map<std::string, EventID> EventLookup;
	EventLookup eventLookup;

public:

	P3DEventCommand(Prepar3D* p3d);
	~P3DEventCommand();
	bool dispatchEvent(EventID event);
	bool dispatchEvent(const std::string& eventName);


};

