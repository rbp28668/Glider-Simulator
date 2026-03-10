#pragma once

class Prepar3D;
class Simulation;

/// <summary>
/// Class to control a winch launch
/// </summary>
class LaunchController
{

	Simulation* pSimulation;
	Prepar3D* pSim;
	bool inProgress = false;
	float startTime = 0.0f;
	float stageStartTime = 0.0f;
	float startingBank = 0.0f;

	enum class Stage {
		IDLE,
		WINGS_LEVEL,
		UP_SLACK,
		GROUND_RUN,
		LAUNCHING
	};

	Stage stage = Stage::IDLE;

	const float WINGS_LEVEL_TIME = 2.0f; // Time to go to wings level (s)
	const float UP_SLACK_TIME = 3.0f;  // Time to wait whilst up-slack
	const float THROTTLE_RAMP_TIME = 2.0f;  // Time to go to full throttle
	const float WING_RELEASE_SPEED = 2.7f;  // a fast jog, time to let go...

	void levelWings(float dt);
	void holdWingsLevel();

public:
	LaunchController(Prepar3D* pSim,  Simulation* pSimulation);
	bool launch(float time);
	void tick(float time);

	bool isLaunching() { return inProgress; }


};

