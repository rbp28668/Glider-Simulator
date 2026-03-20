#include "LaunchController.h"
#include "../P3DCommon/Prepar3D.h"
#include "simulation.h"
#include "SimplePlayer.h"

void LaunchController::levelWings(float stageTime) {
	StateVector<NumberT>& state = pSimulation->get_state();
	auto orientation = state.orientation();
	auto euler = orientation.to_euler(); // as roll, pitch and heading

	// Move to zero over the wings levelling time
	float angleFraction = (WINGS_LEVEL_TIME - stageTime) / WINGS_LEVEL_TIME;

	euler[0] = startingBank * angleFraction;
	orientation = Quaternion<NumberT>::from_euler_angles(euler[0], euler[1], euler[2]);
	state.set_orientation(orientation);

}

void LaunchController::holdWingsLevel()
{
	StateVector<NumberT>& state = pSimulation->get_state();
	auto orientation = state.orientation();
	auto euler = orientation.to_euler(); // as roll, pitch and heading
	euler[0] = 0.0f; // zero roll;
	orientation = Quaternion<NumberT>::from_euler_angles(euler[0], euler[1], euler[2]);

	state.set_orientation(orientation);
}

void LaunchController::showText(const char* lpszText)
{
	::SimConnect_Text(pSim->getHandle(), SIMCONNECT_TEXT_TYPE_PRINT_RED, 5, textEventId, DWORD(1 + strlen(lpszText)) , const_cast<void*>(reinterpret_cast<const void*>(lpszText)));
}

LaunchController::LaunchController(Prepar3D* pSim, Simulation* pSimulation)
	: pSim(pSim)
	, pSimulation(pSimulation)
{
	textEventId = pSim->nextRequestId();
}

bool LaunchController::launch(float time)
{
	if (inProgress) return false;

	startTime = time;
	stageStartTime = time;
	powerFade = false; // if previously set

	StateVector<NumberT>& state = pSimulation->get_state();
	auto orientation = state.orientation();
	auto euler = orientation.to_euler(); // as roll, pitch and heading
	startingBank = euler[0];	// start value for rolling wings level

	stage = Stage::WINGS_LEVEL;
	inProgress = true;

	pSimulation->setup_winch_launch(1500.0f, 10000.0f);
	pSimulation->set_winch_throttle(0);
	std::cout << "Launching" << std::endl;
	showText("Launching");

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
			showText("Take up slack...");
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
				showText("All out...");
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
			StateVector<NumberT>& state = pSimulation->get_state();
			NumberT forward_speed = state.velocity()[0];
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
			showText("Released!");
			std::cout << "Released" << std::endl;
		}
		else {

			if (powerFade) {
				NumberT fraction = 1.0 - (time - powerFadeStartTime) / powerFadeDuration;
				if (fraction < 0) fraction = 0;
				NumberT throttle = powerFadeStartThrottle * fraction;
				pSimulation->set_winch_throttle(throttle);
			}

			// Reduce throttle when cable angle is steep (near top of winch launch)
			float cable_angle = pSimulation->get_winch_cable_angle();
			if (cable_angle > 80.0f * PI / 180.0f) {
				pSimulation->set_winch_throttle(0.2f);
			}

			if (wingdrop) {
				pSimulation->setRollBias(0);
				wingdrop = false;
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
	showText("Released!");
}

void LaunchController::startPowerFade(NumberT startTime, NumberT seconds) {
	powerFade = true;
	powerFadeStartTime = startTime;
	powerFadeDuration = seconds;
	powerFadeStartThrottle = pSimulation->get_winch_throttle();
}

void LaunchController::dropWing(NumberT startTime, bool rollRight) {
	wingdrop = true;
	dropRightWing = rollRight;
	dropStartTime = startTime;

	auto rollBias = (rollRight) ? 8.5 * 50 : 8.5 * -50; // apply c. 2kg force at wingtip.
	pSimulation->setRollBias(rollBias);
}
