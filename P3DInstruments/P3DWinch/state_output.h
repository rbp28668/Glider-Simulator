#pragma once

#include "..\P3DCommon\simobjectdata.h"
#include "state_vector.h"

class Prepar3D;

class StateOutput :public SimObjectData
{

	static DataItem dataItems[];

#pragma pack(push, 1)
	struct Data {

		//SIMCONNECT_DATA_XYZ bodyVelocity; //{"STRUCT BODY VELOCITY", "metres per second", SIMCONNECT_DATATYPE_XYZ},
		//SIMCONNECT_DATA_XYZ bodyAcceleration; //{ "STRUCT BODY ACCELERATION","meters per second squared", SIMCONNECT_DATATYPE_XYZ },
		//SIMCONNECT_DATA_XYZ bodyRotationVelocity; //{ "STRUCT BODY ROTATION VELOCITY","Radians per second",SIMCONNECT_DATATYPE_XYZ },
		//SIMCONNECT_DATA_XYZ bodyRotationAcceleration; //{ "STRUCT BODY ROTATION ACCELERATION","Radians per second squared",SIMCONNECT_DATATYPE_XYZ },

		float pitch;//{ "PLANE PITCH DEGREES","Radians",SIMCONNECT_DATATYPE_FLOAT32 },
		float bank; //{ "PLANE BANK DEGREES", "Radians",SIMCONNECT_DATATYPE_FLOAT32 },
		float heading;//{ "PLANE HEADING DEGREES TRUE","Radians",SIMCONNECT_DATATYPE_FLOAT32 },

		//int32_t onGround; //{ "SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32 },
	};
#pragma pack(pop)

	Data data;

public:

	virtual DataItem* items();

	virtual int itemCount();
		
	StateOutput(Prepar3D*);

	void updateFrom(const StateVector<float>& sv);
};


