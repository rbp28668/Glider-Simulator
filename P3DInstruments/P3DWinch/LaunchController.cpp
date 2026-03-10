#include "LaunchController.h"
#include "../P3DCommon/Prepar3D.h"
#include "simulation.h"
#include "SimplePlayer.h"

void LaunchController::levelWings(float stageTime) {
	StateVector<float>& state = pSimulation->get_state();
	auto orientation = state.orientation();
	auto euler = orientation.to_euler(); // as roll, pitch and heading

	// Move to zero over the wings levelling time
	float angleFraction = (WINGS_LEVEL_TIME - stageTime) / WINGS_LEVEL_TIME;

	euler[0] = startingBank * angleFraction;
	orientation = Quaternion<float>::from_euler_angles(euler[0], euler[1], euler[2]);
	state.set_orientation(orientation);

}

void LaunchController::holdWingsLevel()
{
	StateVector<float>& state = pSimulation->get_state();
	auto orientation = state.orientation();
	auto euler = orientation.to_euler(); // as roll, pitch and heading
	euler[0] = 0.0f; // zero roll;
	orientation = Quaternion<float>::from_euler_angles(euler[0], euler[1], euler[2]);

	state.set_orientation(orientation);
}

LaunchController::LaunchController(Prepar3D* pSim, Simulation* pSimulation)
	: pSim(pSim)
	, pSimulation(pSimulation)
{
}

bool LaunchController::launch(float time)
{
	if (inProgress) return false;

	//if (!pSim->isStarted()) return false;
	//if (pSim->isPaused()) return false;
	//if (!pSim->isScenarioRunning()) return false;

	startTime = time;
	stageStartTime = time;

	StateVector<float>& state = pSimulation->get_state();
	auto orientation = state.orientation();
	auto euler = orientation.to_euler(); // as roll, pitch and heading
	startingBank = euler[0];	// start value for rolling wings level

	stage = Stage::WINGS_LEVEL;
	inProgress = true;

	pSimulation->setup_winch_launch(1500.0f, 10000.0f);
	pSimulation->set_winch_throttle(0);
	std::cout << "Launching" << std::endl;

	return inProgress;
}

void LaunchController::tick(float time)
{
	if (!inProgress) return;

	float dt = time - stageStartTime;

	switch (stage) {
	case Stage::WINGS_LEVEL:
		if (dt < WINGS_LEVEL_TIME) {
			levelWings(dt);
		}
		else {
			stage = Stage::UP_SLACK;
			stageStartTime = time;
			std::cout << "Take up slack" << std::endl;
			pSimulation->engage_winch();
			std::cout << "Winch engaged" << std::endl;

		}
		break;

	case Stage::UP_SLACK:

		if (!pSimulation->is_winch_engaged()) {
			stage = Stage::IDLE;
			inProgress = false;
		}
		else {


			if (dt < UP_SLACK_TIME) {
				holdWingsLevel();
			}
			else {
				stage = Stage::GROUND_RUN;
				stageStartTime = time;
			}
		}
		break;

	case Stage::GROUND_RUN:
		if (!pSimulation->is_winch_engaged()) {
			stage = Stage::IDLE;
			inProgress = false;
		}
		else
		{
			// Smoothly open the throttle
			float throttle = dt / THROTTLE_RAMP_TIME;
			if (throttle > 1.0f) throttle = 1.0f;
			pSimulation->set_winch_throttle(throttle);

			// and hold the wings.
			StateVector<float>& state = pSimulation->get_state();
			float forward_speed = state.velocity()[0];
			if (forward_speed < WING_RELEASE_SPEED) {
				holdWingsLevel();
			}

			// transfer to launching ?  Only if at full power & off the ground
			if (throttle == 1.0f && !pSimulation->is_on_ground()) {
				stage = Stage::LAUNCHING;
				stageStartTime = time;
				std::cout << "Airborne" << std::endl;
			}
		}
		break;

	case Stage::LAUNCHING:

		if (!pSimulation->is_winch_engaged()) {
			// TODO - release noise
			stage = Stage::IDLE;
			inProgress = false;
			std::cout << "Released" << std::endl;
		}
		else {
			// Reduce throttle when cable angle is steep (near top of winch launch)
			float cable_angle = pSimulation->get_winch_cable_angle();
			if (cable_angle > 80.0f * PI / 180.0f) {
				pSimulation->set_winch_throttle(0.2f);
			}
		}
		break;
	}
}

void LaunchController::release()
{
	std::cout << "RELEASE" << std::endl;
	stage = Stage::IDLE;
	inProgress = false;
	pSimulation->release_winch();
}
