
#include <iostream>
#include <sstream>

#include "state_input.h"
#include "state_output.h"
#include "control_inputs.h"
#include "state_vector.h"
#include "simulation.h"



SimObjectData::DataItem StateInput::dataItems[] = {
	// Aircraft state
	{"STRUCT BODY VELOCITY","meters per second", SIMCONNECT_DATATYPE_XYZ},
	{"STRUCT BODY ACCELERATION","meters per second squared", SIMCONNECT_DATATYPE_XYZ},
	{"STRUCT BODY ROTATION VELOCITY","Radians per second",SIMCONNECT_DATATYPE_XYZ},
	{"STRUCT BODY ROTATION ACCELERATION","Radians per second squared",SIMCONNECT_DATATYPE_XYZ},
	{"PLANE PITCH DEGREES","Radians",SIMCONNECT_DATATYPE_FLOAT32},
	{"PLANE BANK DEGREES", "Radians",SIMCONNECT_DATATYPE_FLOAT32},
	{"PLANE HEADING DEGREES TRUE","Radians",SIMCONNECT_DATATYPE_FLOAT32},
	
	// Controls
	{"RUDDER POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT32}, //	Rudder input deflection[-1.0:Full Left, 1.0 : Full Right]	Position	Y -
	{"ELEVATOR POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32}, //	Elevator input deflection[-1.0:Full Down, 1.0 : Full Up]	Position	Y -
	{"AILERON POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32}, // Aileron input left/right [-1.0: Full Left, 1.0: Full Right]
	{"SPOILERS HANDLE POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32}, //Spoiler handle position [0: Retracted, 1.0: Fully Extended]

	{"SIM TIME","Seconds", SIMCONNECT_DATATYPE_FLOAT32}, //	The elapsed simulation time	Seconds

	// World information
	{"AMBIENT WIND X", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //	Wind component in East / West direction.Feet per second	N -
	{"AMBIENT WIND Y", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //		Wind component in vertical direction.Feet per second	N -
	{"AMBIENT WIND Z", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //		Wind component in North / South direction.Feet per second	N -
	{"GROUND ALTITUDE", "meters", SIMCONNECT_DATATYPE_FLOAT32}, //		Altitude of surface	Meters	N	-

	{"SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32},
};

StateInput::StateInput(Prepar3D* p3d, Simulation* pFlightModel) : SimObjectData(p3d), pFlightModel(pFlightModel) {
	createDefinition();

	pOutput = new StateOutput(p3d);
}

SimObjectData::DataItem* StateInput::items() {
	return dataItems;
}

int StateInput::itemCount() {
	return sizeof(dataItems) / sizeof(dataItems[0]);
}

void StateInput::onData(void* pData, SimObject* pObject) {

	struct Data data = *reinterpret_cast<Data*>(pData);
	if (getSim()->isVerbose()) {
		//std::cout << "StateInput Data received" << std::endl;
		//show(pData);
	}

	ControlInputs controls;
	controls.aileron = data.aileron;
	controls.elevator = data.elevator;
	controls.rudder = data.rudder;
	controls.spoiler = data.spoiler;

	World world;  // update world from sim
	world.set_wind_vector(data.windX, data.windY, data.windZ);
	world.set_ground_height(data.ground);

	float dt = (lastSimTime > 0) ? data.time - lastSimTime : 0.1f; 
	lastSimTime = data.time;
	std::cout << dt << std::endl;

	StateVector<float> sv = pFlightModel->update(dt, controls, world);

	pOutput->updateFrom(sv);
}
