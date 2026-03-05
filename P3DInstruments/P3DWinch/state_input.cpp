
#include <iostream>
#include <sstream>
#include <conio.h>

#include "state_input.h"
#include "state_output.h"
#include "control_inputs.h"
#include "state_vector.h"
#include "simulation.h"





SimObjectData::DataItem StateInput::dataItems[] = {
	// Aircraft state - use individual FLOAT32 variables (not STRUCT XYZ) so SimConnect honours unit conversion
	{"VELOCITY BODY X","meters per second", SIMCONNECT_DATATYPE_FLOAT32},	//	True lateral speed, relative to aircraft axis
	{"VELOCITY BODY Y","meters per second", SIMCONNECT_DATATYPE_FLOAT32},	//	True vertical speed, relative to aircraft axis
	{"VELOCITY BODY Z","meters per second", SIMCONNECT_DATATYPE_FLOAT32},	//	True longitudinal speed, relative to aircraft axis
	{"ROTATION VELOCITY BODY X","radians per second", SIMCONNECT_DATATYPE_FLOAT32},	//	Rotation relative to aircraft axis (pitch rate)
	{"ROTATION VELOCITY BODY Y","radians per second", SIMCONNECT_DATATYPE_FLOAT32},	//	Rotation relative to aircraft axis (yaw rate)
	{"ROTATION VELOCITY BODY Z","radians per second", SIMCONNECT_DATATYPE_FLOAT32},	//	Rotation relative to aircraft axis (roll rate)
	{"PLANE PITCH DEGREES","Radians",SIMCONNECT_DATATYPE_FLOAT32},
	{"PLANE BANK DEGREES", "Radians",SIMCONNECT_DATATYPE_FLOAT32},
	{"PLANE HEADING DEGREES TRUE","Radians",SIMCONNECT_DATATYPE_FLOAT32},
	
	{"PLANE LATITUDE","Radians", SIMCONNECT_DATATYPE_FLOAT32}, //	Latitude of aircraft, North is positive, South negative	Radians	Y -
	{"PLANE LONGITUDE","Radians", SIMCONNECT_DATATYPE_FLOAT32}, //	Longitude of aircraft, East is positive, West negative	Radians	Y -
	{"PLANE ALTITUDE","Meters", SIMCONNECT_DATATYPE_FLOAT32}, //	Altitude of aircraft	Feet	Y

	// Controls
	{"RUDDER POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT32}, //	Rudder input deflection[-1.0:Full Left, 1.0 : Full Right]	Position	Y -
	{"ELEVATOR POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32}, //	Elevator input deflection[-1.0:Full Down, 1.0 : Full Up]	Position	Y -
	{"AILERON POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32}, // Aileron input left/right [-1.0: Full Left, 1.0: Full Right]
	{"SPOILERS HANDLE POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT32}, //Spoiler handle position [0: Retracted, 1.0: Fully Extended]
	{"BRAKE LEFT POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT32}, //Brake input [0: Released, 1.0: Full]

	{"SIM TIME","Seconds", SIMCONNECT_DATATYPE_FLOAT32}, //	The elapsed simulation time	Seconds

	// World information
	{"AMBIENT WIND X", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //	Wind component in East / West direction.Feet per second	N -
	{"AMBIENT WIND Y", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //		Wind component in vertical direction.Feet per second	N -
	{"AMBIENT WIND Z", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //		Wind component in North / South direction.Feet per second	N -
	{"GROUND ALTITUDE", "meters", SIMCONNECT_DATATYPE_FLOAT32}, //		Altitude of surface	Meters	N	-

	{"SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32},
};

StateInput::StateInput(Prepar3D* p3d, Simulation* pFlightModel) : SimObjectData(p3d), pFlightModel(pFlightModel),  events(p3d) {
	createDefinition();

	pOutput = new StateOutput(p3d);
}

SimObjectData::DataItem* StateInput::items() {
	return dataItems;
}

int StateInput::itemCount() {
	return sizeof(dataItems) / sizeof(dataItems[0]);
}

void StateInput::engage() {

	events.dispatchEvent(P3DEvent::FREEZE_LATITUDE_LONGITUDE_SET, 1);
	events.dispatchEvent(P3DEvent::FREEZE_ALTITUDE_SET, 1);
	events.dispatchEvent(P3DEvent::FREEZE_ATTITUDE_SET, 1);

	initialised = false;
	engaged = true;
	std::cout << "ENGAGED" << std::endl;
}

void StateInput::disengage() {
	initialised = false;
	engaged = false;

	events.dispatchEvent(P3DEvent::FREEZE_LATITUDE_LONGITUDE_SET, 0);
	events.dispatchEvent(P3DEvent::FREEZE_ALTITUDE_SET, 0);
	events.dispatchEvent(P3DEvent::FREEZE_ATTITUDE_SET, 0);

	std::cout << "DISENGAGED" << std::endl;

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
	controls.brake = data.brake;
	if (_kbhit()) {
		char ch = _getch();

		switch (ch) {
		case 'e':
			engage();
			break;

		case 'd':
			disengage();
			break;

		case 'w':
			if (!data.onGround) {
				std::cout << "You can't winch when you're airborne you muppet" << std::endl;
			}
			else {
				engage();
				// Set up and engage winch for launch
				pFlightModel->setup_winch_launch();
				pFlightModel->engage_winch();
			}
			break;
		default: 
			std::cout << "Unknown command" << std::endl;
		}
	}

	if (!engaged) {
		return;
	}


	if (initialised) {

		ControlInputs controls;
		controls.aileron = data.aileron;
		controls.elevator = data.elevator;
		controls.rudder = data.rudder;
		controls.spoiler = data.spoiler;

		World world;  // update world from sim
		world.set_wind_vector(data.windZ, data.windX, -data.windY); // convert from P3D world (East,Up,North) to NED (North,East,Down)
		world.set_ground_height(-data.ground); // convert altitude (positive up) to NED Z (positive down)

		float dt = data.time - lastSimTime;
		lastSimTime = data.time;

		StateVector<float> sv = pFlightModel->update(dt, controls, world);

		auto pos = sv.position(); // Position in meters NED
		pOutput->data.latitude = start_lat + pos[0] / metresPerRadianLat;  // North
		pOutput->data.longitude = start_lon + pos[1] / metresPerRadianLon;  // East
		pOutput->data.altitude = -pos[2]; // Down

		auto orientation = sv.orientation();
		auto rpy = orientation.to_euler();  // as roll, pitch and yaw

		pOutput->data.bank = -rpy[0];	// NED right-bank positive → P3D left-bank positive
		pOutput->data.pitch = -rpy[1];	// NED nose-up positive → P3D nose-down positive
		pOutput->data.heading = rpy[2];

		auto v = sv.velocity();
		pOutput->data.velocity_body_z = v[0];
		pOutput->data.velocity_body_x = v[1];
		pOutput->data.velocity_body_y = -v[2];

		auto linear_acceleration = pFlightModel->get_linear_acceleration();
		pOutput->data.acceleration_body_z = linear_acceleration[0];
		pOutput->data.acceleration_body_x = linear_acceleration[1];
		pOutput->data.acceleration_body_y = -linear_acceleration[2];


		// Angular velocity/acceleration are pseudovectors: signs flip vs polar vectors
		// because P3D↔NED transform has det=-1 (LH↔RH reflection)
		auto av = sv.angular_velocity();
		pOutput->data.rotation_body_z = -av[0];
		pOutput->data.rotation_body_x = -av[1];
		pOutput->data.rotation_body_y = av[2];

		auto angular_acceleration = pFlightModel->get_angular_acceleration();
		pOutput->data.rotation_acceleration_body_z = -angular_acceleration[0];
		pOutput->data.rotation_acceleration_body_x = -angular_acceleration[1];
		pOutput->data.rotation_acceleration_body_y = angular_acceleration[2];

		pOutput->sendData();
	}
	else { // not initialised

		StateVector<float>& state = pFlightModel->get_state();
		state.set_orientation(Quaternion<float>::from_euler_angles(-data.bank, -data.pitch, data.heading)); // negate bank & pitch: P3D LH→NED RH
		state.set_position( 0.0f, 0.0f, -data.altitude );
		state.set_velocity(data.velocity_body_z, data.velocity_body_x, -data.velocity_body_y); // polar vector: convert from P3D to NED
		state.set_angular_velocity(-data.rotation_velocity_body_z, -data.rotation_velocity_body_x, data.rotation_velocity_body_y); // pseudovector: signs flip vs polar
		start_lat = data.latitude;
		start_lon = data.longitude;
		
		constexpr float earthEquatorialRadius = 6378.1f * 1000.0f; //  metres
		constexpr float earthPolarRadius = 6356.8f * 1000.0f; // metres
		const float pi = 3.14159265358979f;

		metresPerRadianLat = earthPolarRadius;   // polar circumference / 360
		metresPerRadianLon = earthEquatorialRadius * std::cos(start_lat);


		lastSimTime = data.time;



		initialised = true;
	}

}
