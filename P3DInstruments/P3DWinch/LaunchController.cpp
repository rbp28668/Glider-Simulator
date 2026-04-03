#include "LaunchController.h"
#include "../P3DCommon/Prepar3D.h"
#include "simulation.h"

void LaunchController::setResourceFolder(const Directory& folder) {
	resourceFolder = folder;
}


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

	stage = Stage::SETTLE;
	inProgress = true;

	pSimulation->setup_winch_launch(1200.0f, 10000.0f); // 22 length, black link
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
	case Stage::SETTLE:
		if (dt < SETTLE_TIME) {
			// NOP
		}
		else {
			stage = Stage::CABLE_ON;
			stageStartTime = time;
			File file = resourceFolder.file("cable_on_and_secure_black_link.m4a");
			player.Play(file);
		}
		break;

	case Stage::CABLE_ON:    // short delay whilst saying cable on before wings start to level.
		if (dt < CABLE_ON_TIME) {
			// NOP
		}
		else {
			stage = Stage::WINGS_LEVEL;
			stageStartTime = time;
		}
		break;

	case Stage::WINGS_LEVEL:
		if (dt < WINGS_LEVEL_TIME) {
			levelWings(dt);
		}
		else {
			stage = Stage::UP_SLACK;
			stageStartTime = time;
			std::cout << "Take up slack" << std::endl;
			showText("Take up slack...");
			File file = resourceFolder.file("take_up_slack.m4a");
			player.Play(file);


			pSimulation->engage_winch();
			pSimulation->set_winch_throttle(0);
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
				File file = resourceFolder.file("all_out.m4a");
				player.Play(file);
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
			// Smoothly open the throttle to targetThrottle.
			NumberT throttle = targetThrottle * dt / THROTTLE_RAMP_TIME;
			if (throttle > targetThrottle) throttle = targetThrottle;
			pSimulation->set_winch_throttle(throttle);

			// and hold the wings.
			StateVector<NumberT>& state = pSimulation->get_state();
			NumberT forward_speed = state.velocity()[0];
			if (forward_speed < WING_RELEASE_SPEED) {
				holdWingsLevel();
			}

			// transfer to launching ?  Only if at full power & off the ground
			if (throttle == targetThrottle && !pSimulation->is_on_ground()) {
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
			pSimulation->setRollBias(0);
			showText("Released!");
			std::cout << "Released" << std::endl;
		}
		else {

			if (powerFade) {
				NumberT fraction = targetThrottle - (time - powerFadeStartTime) / powerFadeDuration;
				if (fraction < 0) fraction = 0;
				NumberT throttle = powerFadeStartThrottle * fraction;
				pSimulation->set_winch_throttle(throttle);
			}
			else {
				pSimulation->set_winch_throttle(targetThrottle);
			}

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
	showText("Released!");
}

void LaunchController::startPowerFade(NumberT startTime, NumberT seconds) {
	powerFade = true;
	powerFadeStartTime = startTime;
	powerFadeDuration = seconds;
	powerFadeStartThrottle = pSimulation->get_winch_throttle();
}

void LaunchController::dropWing(NumberT startTime, bool rollRight) {
	NumberT tipForce = 20; // Newtons
	
	auto rollBias = (rollRight) ? 8.5 * tipForce : 8.5 * -tipForce; // apply force at wingtip.
	pSimulation->setRollBias(rollBias);
}


NumberT LaunchController::adjustPower(NumberT amount)
{
	targetThrottle += amount;
	if (targetThrottle < 0) targetThrottle = 0;
	else if (targetThrottle > 1.0) targetThrottle = 1.0;
	return targetThrottle;
}

void LaunchController::setPower(NumberT amount)
{
	targetThrottle = amount;
	if (targetThrottle < 0) targetThrottle = 0;
	else if (targetThrottle > 1.0) targetThrottle = 1.0;
}
