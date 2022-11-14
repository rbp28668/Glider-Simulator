#pragma once
#include <map>
#include "../P3DCommon/Prepar3D.h"

// Call basic P3D commands via client events.
class P3DEventCommand
{
public:
	typedef enum EVENT_ID {
		START_VALUE = Prepar3D::EVENT_ID::LAST_P3D_EVENT, // Note that without this we get duplicate event ID exeptions. 

		THROTTLE_FULL,			// Set throttles max	Shared Cockpit
		THROTTLE_INCR,			// Increment throttles	Shared Cockpit
		THROTTLE_INCR_SMALL,	// Increment throttles small	Shared Cockpit
		THROTTLE_DECR,			// Decrement throttles	Shared Cockpit
		THROTTLE_DECR_SMALL,	// Decrease throttles small	Shared Cockpit
		THROTTLE_CUT,			// Set throttles to idle	Shared Cockpit
		INCREASE_THROTTLE,		// Increment throttles	Shared Cockpit
		DECREASE_THROTTLE,		// Decrement throttles	Shared Cockpit
		THROTTLE_SET,			// Set throttles exactly(0 - 16383)	Shared Cockpit		
		AXIS_THROTTLE_SET,		// Set throttles (0- 16383)	Shared Cockpit. Note range -16383 to +16383
		AXIS_MIXTURE_SET,		// Set mixture lever 1 exact value(-16383 to + 16383)	Shared Cockpit
		AXIS_PROPELLER_SET,		// Set propeller levers exact value(-16383 to + 16383)	Shared Cockpit

		MIXTURE_SET,			// Set mixture levers to exact value(0 to 16383)	Shared Cockpit
		MIXTURE_RICH,			// Set mixture levers to max rich	Shared Cockpit
		MIXTURE_INCR,			// Increment mixture levers	Shared Cockpit
		MIXTURE_INCR_SMALL,		// Increment mixture levers small	Shared Cockpit
		MIXTURE_DECR,			// Decrement mixture levers	Shared Cockpit
		MIXTURE_DECR_SMALL,		// Decrement mixture levers small	Shared Cockpit
		MIXTURE_LEAN,			// Set mixture levers to max lean	Shared Cockpit
		MIXTURE_SET_BEST,		// Set mixture levers to current best power setting	Shared Cockpit

		PROP_PITCH_SET,			// Set prop pitch levers(0 to 16383)	Shared Cockpit
		PROP_PITCH_LO,			// Set prop pitch levers max(lo pitch)	Shared Cockpit
		PROP_PITCH_INCR,		// Increment prop pitch levers	Shared Cockpit
		PROP_PITCH_INCR_SMALL,	// Increment prop pitch levers small	Shared Cockpit
		PROP_PITCH_DECR,		// Decrement prop pitch levers	Shared Cockpit
		PROP_PITCH_DECR_SMALL,	// Decrease prop levers small	Shared Cockpit
		PROP_PITCH_HI,			// Set prop pitch levers min(hi pitch)	Shared Cockpit

		MAGNETO_OFF,			// Set engine magnetos off	Shared Cockpit
		MAGNETO_RIGHT,			// Set engine right magnetos on	Shared Cockpit
		MAGNETO_LEFT,			// Set engine left magnetos on	Shared Cockpit
		MAGNETO_BOTH,			// Set engine magnetos on	Shared Cockpit
		MAGNETO_START,			// Set engine magnetos on and toggle starters	Shared Cockpit
		ENGINE_AUTO_START,		// Triggers auto - start	Shared Cockpit
		ENGINE_AUTO_SHUTDOWN,	// Triggers auto - shutdown	Shared Cockpit
		ENGINE,					// Sets engines for 1, 2, 3, 4 selection(to be followed by SELECT_n)	Shared Cockpit
		FLAPS_UP,
		FLAPS_1,
		FLAPS_2,
		FLAPS_3,
		FLAPS_DOWN,
		ELEV_TRIM_DN,
		ELEV_TRIM_UP,
		SPOILERS_ON,
		SPOILERS_OFF,
		ELEVATOR_SET,			// Sets elevator position (-16383 - +16383)
		AILERON_SET,			// Sets aileron position (-16383 - +16383)
		RUDDER_SET,				// Sets rudder position (-16383 - +16383)
		BAROMETRIC,
		KOHLSMAN_INC,
		KOHLSMAN_DEC,
		KOHLSMAN_SET,
		RESET_G_FORCE_INDICATOR,
		TOGGLE_VACUUM_FAILURE,			// Toggle vacuum system failure	Shared Cockpit
		TOGGLE_ELECTRICAL_FAILURE,		// Toggle electrical system failure	Shared Cockpit
		TOGGLE_PITOT_BLOCKAGE,			// Toggles blocked pitot tube	Shared Cockpit
		TOGGLE_STATIC_PORT_BLOCKAGE,	// Toggles blocked static port	Shared Cockpit
		TOGGLE_HYDRAULIC_FAILURE,		// Toggles hydraulic system failure	Shared Cockpit
		TOGGLE_TOTAL_BRAKE_FAILURE,		// Toggles brake failure(both)	Shared Cockpit
		TOGGLE_ENGINE1_FAILURE,
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
		SELECT_1,			// Sets "selected" index(for other events) to 1	Shared Cockpit
		SELECT_2,			// Sets "selected" index(for other events) to 2	Shared Cockpit
		SELECT_3,			// Sets "selected" index(for other events) to 3	Shared Cockpit
		SELECT_4,			// Sets "selected" index(for other events) to 4	Shared Cockpit
		MINUS,				// Used in conjunction with "selected" parameters to decrease their value(e.g., radio frequency)	Shared Cockpit
		PLUS,				// Used in conjunction with "selected" parameters to increase their value(e.g., radio frequency)
		SITUATION_SAVE,
		SITUATION_RESET,
		TOGGLE_AIRCRAFT_LABELS, 
		EXIT,
		ABORT,
		SIM_RESET,
		REFRESH_SCENERY,
		CAPTURE_SCREENSHOT,
		AUTORUDDER_TOGGLE, 
		FREEZE_LATITUDE_LONGITUDE_TOGGLE, //Turns the freezing of the lat/lon position of the aircraft (either user or AI controlled) on or off. 
		FREEZE_LATITUDE_LONGITUDE_SET, // 	Freezes the lat/lon position of the aircraft.
		FREEZE_ALTITUDE_TOGGLE, //	Turns the freezing of the altitude of the aircraft on or off.
		FREEZE_ALTITUDE_SET, // Freezes the altitude of the aircraft..
		FREEZE_ATTITUDE_TOGGLE, // Turns the freezing of the attitude (pitch, bank and heading) of the aircraft on or off.
		FREEZE_ATTITUDE_SET, // Freezes the attitude (pitch, bank and heading) of the aircraft.
		POSITION_FREEZE_USER, // Toggles the position freeze (Lat/Lon, Altitude and Attitude) of the controlled SimObject.
		POSITION_FREEZE_ALL, // Toggles the position freeze(Lat / Lon, Altitudeand Attitude) of all SimObjects.
		SLEW_TOGGLE,	// Toggles slew on / off	Shared Cockpit(Pilot only)
		SLEW_OFF,		// Turns slew off	Shared Cockpit(Pilot only)
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
	bool dispatchEvent(EventID event, DWORD  dwData, DWORD dwId = SIMCONNECT_OBJECT_ID_USER);
	bool dispatchEvent(const std::string& eventName, DWORD  dwData, DWORD dwId = SIMCONNECT_OBJECT_ID_USER);


};

