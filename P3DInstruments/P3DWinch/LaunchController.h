#pragma once

#include "sim_types.h"
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
	NumberT startingBank = 0.0f;
	unsigned long textEventId; // SIMCONNECT_CLIENT_EVENT_ID
	
	bool powerFade = false;
	NumberT powerFadeStartTime = 0.0;
	NumberT powerFadeDuration = 0.0;
	NumberT powerFadeStartThrottle = 0.0;

	bool wingdrop;
	bool dropRightWing;
	NumberT dropStartTime;

	enum class Stage {
		IDLE,
		WINGS_LEVEL,
		UP_SLACK,
		GROUND_RUN,
		LAUNCHING
	};

	Stage stage = Stage::IDLE;

	const float WINGS_LEVEL_TIME = 2.0f; // Time to go to wings level (s)
	const float UP_SLACK_TIME = 1.0f;  // Time to wait whilst up-slack
	const float THROTTLE_RAMP_TIME = 5.0f;  // Time to go to full throttle
	const float WING_RELEASE_SPEED = 2.7f;  // a fast jog, time to let go...

	void levelWings(float dt);
	void holdWingsLevel();

	void showText(const char* lpszText);

public:
	LaunchController(Prepar3D* pSim,  Simulation* pSimulation);
	bool launch(float time);
	void tick(float time);
	bool isLaunching() { return inProgress; }
	void release();
	void startPowerFade(NumberT startTime, NumberT seconds = 10.0);
	void dropWing(NumberT startTime, bool rollRight);

};

