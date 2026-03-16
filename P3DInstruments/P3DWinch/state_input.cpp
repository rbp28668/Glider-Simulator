
#include <iostream>
#include <sstream>
#include <conio.h>

#include "state_input.h"
#include "state_output.h"
#include "control_inputs.h"
#include "state_vector.h"
#include "simulation.h"
#include "Timer.h"





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
	{"TOW RELEASE HANDLE","Position", SIMCONNECT_DATATYPE_FLOAT32},  //Position of tow release handle. 100 is fully deployed.	Percent over 100	N

	{"SIM TIME","Seconds", SIMCONNECT_DATATYPE_FLOAT64}, //	The elapsed simulation time	Seconds

	// World information
	{"AMBIENT WIND X", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //	Wind component in East / West direction.Feet per second	N -
	{"AMBIENT WIND Y", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //		Wind component in vertical direction.Feet per second	N -
	{"AMBIENT WIND Z", "meters per second", SIMCONNECT_DATATYPE_FLOAT32}, //		Wind component in North / South direction.Feet per second	N -
	{"GROUND ALTITUDE", "meters", SIMCONNECT_DATATYPE_FLOAT32}, //		Altitude of surface	Meters	N	-

	{"SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32},
};

StateInput::StateInput(Prepar3D* p3d, Simulation* pFlightModel) : SimObjectData(p3d)
, pFlightModel(pFlightModel)
, events(p3d) 
, launcher(p3d, pFlightModel)
{
	createDefinition();

	pOutput = new StateOutput(p3d);
}


// Initialises the model state from the input data so the alternative flight model is going in the same direction at the same place etc.
// Critically, also initialises lastSimTime so the first subsequent call to tick() has a valid time period.
void StateInput::initialiseModel(const Data& data)
{
	StateVector<NumberT>& state = pFlightModel->get_state();
	state.set_orientation(Quaternion<NumberT>::from_euler_angles(-data.bank, -data.pitch, data.heading)); // negate bank & pitch: P3D LH→NED RH
	state.set_position(0.0f, 0.0f, -(data.altitude + pFlightModel->zOffset()));
	state.set_velocity(data.velocity_body_z, data.velocity_body_x, -data.velocity_body_y); // polar vector: convert from P3D to NED
	state.set_angular_velocity(-data.rotation_velocity_body_z, -data.rotation_velocity_body_x, data.rotation_velocity_body_y); // pseudovector: signs flip vs polar
	start_lat = data.latitude;
	start_lon = data.longitude;

	constexpr NumberT earthEquatorialRadius = 6378.1f * 1000.0f; //  metres
	constexpr NumberT earthPolarRadius = 6356.8f * 1000.0f; // metres
	const float pi = 3.14159265358979f;

	metresPerRadianLat = earthPolarRadius;   // polar circumference / 360
	metresPerRadianLon = earthEquatorialRadius * std::cos(start_lat);


	lastSimTime = data.time;

	initialised = true;

	

}

void StateInput::tickModel(const Data& data)
{

	ControlInputs controls;
	controls.aileron = data.aileron;
	controls.elevator = data.elevator;
	controls.rudder = data.rudder;
	controls.spoiler = data.spoiler;
	controls.brake = data.brake;
	controls.release = data.release;

	World world;  // update world from sim
	world.set_wind_vector(data.windZ, data.windX, -data.windY); // convert from P3D world (East,Up,North) to NED (North,East,Down)
	world.set_ground_height(-data.ground); // convert altitude (positive up) to NED Z (positive down)

	NumberT dt = data.time - lastSimTime;
	lastSimTime = data.time;

	StateVector<NumberT> sv = pFlightModel->update(dt, controls, world);

	StateOutput::Data* pData = pOutput->getData();
	auto pos = sv.position(); // Position in meters NED
	pData->latitude = start_lat + pos[0] / metresPerRadianLat;  // North
	pData->longitude = start_lon + pos[1] / metresPerRadianLon;  // East
	pData->altitude = -pos[2]; // Down

	auto orientation = sv.orientation();
	auto rpy = orientation.to_euler();  // as roll, pitch and yaw

	pData->bank = -rpy[0];	// NED right-bank positive → P3D left-bank positive
	pData->pitch = -rpy[1];	// NED nose-up positive → P3D nose-down positive
	pData->heading = rpy[2];

	auto v = sv.velocity();
	pData->velocity_body_z = v[0];
	pData->velocity_body_x = v[1];
	pData->velocity_body_y = -v[2];

	auto linear_acceleration = pFlightModel->get_linear_acceleration();
	pData->acceleration_body_z = linear_acceleration[0];
	pData->acceleration_body_x = linear_acceleration[1];
	pData->acceleration_body_y = -linear_acceleration[2];


	// Angular velocity/acceleration are pseudovectors: signs flip vs polar vectors
	// because P3D↔NED transform has det=-1 (LH↔RH reflection)
	auto av = sv.angular_velocity();
	pData->rotation_body_z = -av[0];
	pData->rotation_body_x = -av[1];
	pData->rotation_body_y = av[2];

	auto angular_acceleration = pFlightModel->get_angular_acceleration();
	pData->rotation_acceleration_body_z = -angular_acceleration[0];
	pData->rotation_acceleration_body_x = -angular_acceleration[1];
	pData->rotation_acceleration_body_y = angular_acceleration[2];

	// optional small delay before we send the data.
	if (useDelay) ::Sleep(5);

	pOutput->sendData();

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



	// Make sure these are serialised
	CriticalSection::Lock lock(criticalSection);

	Timer t;
	auto start = t.raw();

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
				std::cout << "You can't winch when you're airborne" << std::endl;
			}
			else {
				engage();
				winch_launch_pending = true;  // defer until state is initialised
			}
			break;

		case 's':
			if (spinKit) {
				pFlightModel->set_spin_kit(0);
				spinKit = false;
				std::cout << "Spin kit removed" << std::endl;
			}
			else {
				pFlightModel->set_spin_kit(20);  // bolt 20kg of lead to the tail.
				spinKit = true;
				std::cout << "Spin kit attached" << std::endl;
			}
			break;

		default:
			std::cout << "Unknown command" << std::endl;
		}
	}

	// Start a launch?
	if (data.onGround && ! launcher.isLaunching()) {
		if (data.release > 0.5) {
			releasePulled = true;
		}
		else { // not pulled
			if (releasePulled) {  // if it was....
				// Start the launch
				engage();
				winch_launch_pending = true;  // defer until state is initialised
				releasePulled = false;
			}
		}
	}

	// only continue on if actually engaged
	if (!engaged) {
		return;
	}

	// Guards against invalid sim state
	//if (!getSim()->isStarted()) return;
	if (getSim()->isPaused()) return;
	if (getSim()->isCrashed()) {
		disengage();
		return;
	}


	if (initialised) {
		// In the middle of a winch launch?
		if (launcher.isLaunching()) {
			if (data.release > 0.5f) {
				launcher.release();
			}
			launcher.tick(float(data.time));
			if (!launcher.isLaunching() && autoDisengage) {
				disengage(); // auto disengage
			}
		}

		//auto tickStart = t.raw();
		
		tickModel(data);
		
		//auto modelTime = t.since(tickStart);
		//std::cout << "T:" << modelTime * 1000000 << "us" << std::endl;
	}
	else { // not initialised, so initialise - also sets clock so next tick will have valid dt.
		initialiseModel(data);
		
		if (winch_launch_pending) {
			winch_launch_pending = false;
			launcher.launch(float(data.time));
		}
	}

}
