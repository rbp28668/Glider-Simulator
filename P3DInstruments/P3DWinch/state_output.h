#pragma once

#include "..\P3DCommon\simobjectdata.h"
#include "sim_types.h"
#include "state_vector.h"

class Prepar3D;

class StateOutput :public SimObjectData
{

	static DataItem dataItems[];

public:
#pragma pack(push, 1)
	struct Data {

		//SIMCONNECT_DATA_XYZ bodyVelocity; //{"STRUCT BODY VELOCITY", "metres per second", SIMCONNECT_DATATYPE_XYZ},
		//SIMCONNECT_DATA_XYZ bodyAcceleration; //{ "STRUCT BODY ACCELERATION","meters per second squared", SIMCONNECT_DATATYPE_XYZ },
		//SIMCONNECT_DATA_XYZ bodyRotationVelocity; //{ "STRUCT BODY ROTATION VELOCITY","Radians per second",SIMCONNECT_DATATYPE_XYZ },
		//SIMCONNECT_DATA_XYZ bodyRotationAcceleration; //{ "STRUCT BODY ROTATION ACCELERATION","Radians per second squared",SIMCONNECT_DATATYPE_XYZ },

		NumberT velocity_body_x; //{"VELOCITY BODY X", "meters per second", SIMCONNECT_DATATYPE_FLOAT32};//	True lateral speed, relative to aircraft axis	Feet per second	Y -
		NumberT velocity_body_y; //{"VELOCITY BODY Y", "meters per second", SIMCONNECT_DATATYPE_FLOAT32};//	True vertical speed, relative to aircraft axis	Feet per second	Y -
		NumberT velocity_body_z; // {"VELOCITY BODY Z", "meters per second", SIMCONNECT_DATATYPE_FLOAT32};//	True longitudinal speed, relative to aircraft axis	Feet per second	Y -
		NumberT acceleration_body_x;  //{"ACCELERATION BODY X", "meters per second squared", SIMCONNECT_DATATYPE_FLOAT32};//	Acceleration relative to aircraft axis, in east / west direction	Feet per second squared	Y -
		NumberT acceleration_body_y; //{"ACCELERATION BODY Y", "meters per second squared", SIMCONNECT_DATATYPE_FLOAT32};//	Acceleration relative to aircraft axis, in vertical direction	Feet per second squared	Y -
		NumberT acceleration_body_z; //{"ACCELERATION BODY Z", "meters per second squared", SIMCONNECT_DATATYPE_FLOAT32};//	Acceleration relative to aircraft axis, in north / south direction	Feet per second squared	Y -
		NumberT rotation_body_x;  //{"ROTATION VELOCITY BODY X", "radians per second", SIMCONNECT_DATATYPE_FLOAT32};//	Rotation relative to aircraft axis	Radians per second	Y -
		NumberT rotation_body_y; //{"ROTATION VELOCITY BODY Y", "radians per second", SIMCONNECT_DATATYPE_FLOAT32};//	Rotation relative to aircraft axis	Radians per second	Y -
		NumberT rotation_body_z; //{"ROTATION VELOCITY BODY Z", "radians per second", SIMCONNECT_DATATYPE_FLOAT32};//	Rotation relative to aircraft axis	Radians per second	Y -
		NumberT rotation_acceleration_body_x; // {"ROTATION ACCELERATION BODY X", "radians per second", SIMCONNECT_DATATYPE_FLOAT32};//	Rotation acceleration relative to aircraft axis	Radians per second	Y -
		NumberT rotation_acceleration_body_y; //{"ROTATION ACCELERATION BODY Y", "radians per second", SIMCONNECT_DATATYPE_FLOAT32};//	Rotation acceleration relative to aircraft axis	Radians per second	Y -
		NumberT rotation_acceleration_body_z; //{"ROTATION ACCELERATION BODY Z", "radians per second", SIMCONNECT_DATATYPE_FLOAT32};//	Rotation acceleration relative to aircraft axis	Radians per second	Y -

		NumberT latitude; // {"PLANE LATITUDE", "Radians", SIMCONNECT_DATATYPE_FLOAT32}, //	Latitude of aircraft, North is positive, South negative	Radians	Y -
		NumberT longitude; // { "PLANE LONGITUDE", "Radians", SIMCONNECT_DATATYPE_FLOAT32 }, //	Longitude of aircraft, East is positive, West negative	Radians	Y -
		NumberT altitude; // { "PLANE ALTITUDE", "Meters", SIMCONNECT_DATATYPE_FLOAT32 }, //	Altitude of aircraft	Feet	Y

		NumberT pitch;//{ "PLANE PITCH DEGREES","Radians",SIMCONNECT_DATATYPE_FLOAT32 },
		NumberT bank; //{ "PLANE BANK DEGREES", "Radians",SIMCONNECT_DATATYPE_FLOAT32 },
		NumberT heading;//{ "PLANE HEADING DEGREES TRUE","Radians",SIMCONNECT_DATATYPE_FLOAT32 },

		//int32_t onGround; //{ "SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32 },
	};
#pragma pack(pop)

	Data data;

public:

	virtual DataItem* items();

	virtual int itemCount();
		
	StateOutput(Prepar3D*);

	void sendData();
};


