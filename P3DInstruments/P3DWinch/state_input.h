#pragma once

#include "..\P3DCommon\simobjectdata.h"
#include "P3DEvent.h"
#include "LaunchController.h"

class Prepar3D;
class Simulation;
class StateOutput;

// Get state data from the simulator
class StateInput :public SimObjectData
{

	static DataItem dataItems[];

#pragma pack(push, 1)
	struct Data {

		// Body velocity components (m/s) - individual FLOAT32 so SimConnect converts units
		float velocity_body_x;		// lateral speed (right positive)
		float velocity_body_y;		// vertical speed (up positive)
		float velocity_body_z;		// longitudinal speed (forward positive)
		// Body rotation velocity components (rad/s)
		float rotation_velocity_body_x;	// pitch rate
		float rotation_velocity_body_y;	// yaw rate
		float rotation_velocity_body_z;	// roll rate
		
		float pitch;//{ "PLANE PITCH DEGREES","Radians",SIMCONNECT_DATATYPE_FLOAT32 },
		float bank; //{ "PLANE BANK DEGREES", "Radians",SIMCONNECT_DATATYPE_FLOAT32 },
		float heading;//{ "PLANE HEADING DEGREES TRUE","Radians",SIMCONNECT_DATATYPE_FLOAT32 },
	
		float latitude; // {"PLANE LATITUDE", "Radians", SIMCONNECT_DATATYPE_FLOAT32}, //	Latitude of aircraft, North is positive, South negative	Radians	Y -
		float longitude; // { "PLANE LONGITUDE", "Radians", SIMCONNECT_DATATYPE_FLOAT32 }, //	Longitude of aircraft, East is positive, West negative	Radians	Y -
		float altitude; // { "PLANE ALTITUDE", "Meters", SIMCONNECT_DATATYPE_FLOAT32 }, //	Altitude of aircraft	Feet	Y

		float rudder;   //{ "RUDDER POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT32 }, //	Rudder input deflection[-1.0:Full Left, 1.0 : Full Right]	Position	Y -
		float elevator; //{ "ELEVATOR POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32 }, //	Elevator input deflection[-1.0:Full Down, 1.0 : Full Up]	Position	Y -
		float aileron;  //{ "AILERON POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32 }, // Aileron input left/right [-1.0: Full Left, 1.0: Full Right]
		float spoiler;  //{ "SPOILERS HANDLE POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32 }, //Spoiler handle position [0: Retracted, 1.0: Fully Extended]
		float brake;    //{ "BRAKE LEFT POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT32 }, //Brake input [0: Released, 1.0: Full]
		float release;  // {"TOW RELEASE HANDLE","Position", SIMCONNECT_DATATYPE_FLOAT32},  //Position of tow release handle. 100 is fully deployed.	Percent over 100	N

		float time; // {"SIM TIME", "Seconds", SIMCONNECT_DATATYPE_FLOAT32}, //	The elapsed simulation time	Seconds

	    // World information
		float windX; // { "AMBIENT WIND X", "meters per second", SIMCONNECT_DATATYPE_FLOAT32 }, //	Wind component in East / West direction.Feet per second	N -
		float windY; // { "AMBIENT WIND Y", "meters per second", SIMCONNECT_DATATYPE_FLOAT32 }, //		Wind component in vertical direction.Feet per second	N -
		float windZ; // { "AMBIENT WIND Z", "meters per second", SIMCONNECT_DATATYPE_FLOAT32 }, //		Wind component in North / South direction.Feet per second	N -
		float ground; // { "GROUND ALTITUDE", "meters per second", SIMCONNECT_DATATYPE_FLOAT32 }, //		Altitude of surface	Meters	N	-

		int32_t onGround; //{ "SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32 },
	};
#pragma pack(pop)

	Simulation* pFlightModel;
	StateOutput* pOutput;
	P3DEvent events;
	LaunchController launcher;

	float lastSimTime = 0.0f;
	bool initialised = false;
	bool engaged = false;  // set true if should be controlling the sim.
	bool winch_launch_pending = false;  // deferred until state is initialised
	bool releasePulled = false;
	bool spinKit = false;

	float start_lat;
	float start_lon;
	float metresPerRadianLat;
	float metresPerRadianLon;

	void engage();
	void disengage();

	void initialiseModel(const Data& data);
	void tickModel(const Data& data);

public:

	virtual DataItem* items();

	virtual int itemCount();

	virtual void onData(void* pData, SimObject* pObject);

	StateInput(Prepar3D*, Simulation* pFlightModel);

};

