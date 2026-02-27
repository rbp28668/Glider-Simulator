#include "state_output.h"
#include <iostream>

SimObjectData::DataItem StateOutput::dataItems[] = {
	//{"STRUCT BODY VELOCITY","meters per second", SIMCONNECT_DATATYPE_XYZ},
	//{"STRUCT BODY ACCELERATION","meters per second squared", SIMCONNECT_DATATYPE_XYZ},
	//{"STRUCT BODY ROTATION VELOCITY","Radians per second",SIMCONNECT_DATATYPE_XYZ},
	//{"STRUCT BODY ROTATION ACCELERATION","Radians per second squared",SIMCONNECT_DATATYPE_XYZ},

	{"PLANE PITCH DEGREES","Radians",SIMCONNECT_DATATYPE_FLOAT32},
	{"PLANE BANK DEGREES", "Radians",SIMCONNECT_DATATYPE_FLOAT32},
	{"PLANE HEADING DEGREES TRUE","Radians",SIMCONNECT_DATATYPE_FLOAT32},

	//{"SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32},
};

StateOutput::StateOutput(Prepar3D* p3d) : SimObjectData(p3d) {
	createDefinition();
}

SimObjectData::DataItem* StateOutput::items() {
	return dataItems;
}

int StateOutput::itemCount() {
	return sizeof(dataItems) / sizeof(dataItems[0]);
}

void StateOutput::updateFrom(const StateVector<float>& sv) {

	auto attitude = sv.Attitude();
	data.bank = attitude[0];
	data.pitch = attitude[1];
	data.heading = attitude[2];

	std::cout << data.bank << ", " << data.pitch << ", " << data.heading << std::endl;

	//auto velocity = sv.velocity();
	//data.bodyVelocity.x = velocity[0];
	//data.bodyVelocity.y = velocity[1];
	//data.bodyVelocity.z = velocity[2];

	//auto av = sv.angular_velocity();
	//data.bodyRotationVelocity.x = av[0];
	//data.bodyRotationVelocity.y = av[1];
	//data.bodyRotationVelocity.z = av[2];

	//send(&data, sizeof(data), SIMCONNECT_OBJECT_ID_USER);
}
