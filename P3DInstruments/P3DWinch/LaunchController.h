#pragma once

#include "../P3DCommon/Folder.h"
#include "SimplePlayer.h"
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
	
	// target position whilst winching.  Doesn't mean that the throttle position is set to 
	// this at any given point in time but it's where it will be during the majority of the launch.
	NumberT targetThrottle = 0.8;

	bool powerFade = false;
	NumberT powerFadeStartTime = 0.0;
	NumberT powerFadeDuration = 0.0;
	NumberT powerFadeStartThrottle = 0.0;

	enum class Stage {
		IDLE,
		SETTLE,     // Settling after launch initiated
		CABLE_ON,
		WINGS_LEVEL,
		UP_SLACK,
		GROUND_RUN,
		LAUNCHING
	};

	Stage stage = Stage::IDLE;

	const float SETTLE_TIME = 4.0f;    // time to settle after control passes to this.
	const float CABLE_ON_TIME = 3.0f;    // Initial verify cable on.
	const float WINGS_LEVEL_TIME = 2.0f; // Time to go to wings level (s)
	const float UP_SLACK_TIME = 5.0f;  // Time to wait whilst up-slack
	const float THROTTLE_RAMP_TIME = 5.0f;  // Time to go to full throttle
	const float WING_RELEASE_SPEED = 2.7f;  // a fast jog, time to let go...

	Directory resourceFolder;
	SimplePlayer player;  // of sound effects

	void levelWings(float dt);
	void holdWingsLevel();

	void showText(const char* lpszText);



public:
	LaunchController(Prepar3D* pSim,  Simulation* pSimulation);
	void setResourceFolder(const Directory& folder);
	bool launch(float time);
	void tick(float time);
	bool isLaunching() { return inProgress; }
	void release();
	void startPowerFade(NumberT startTime, NumberT seconds = 10.0);
	void dropWing(NumberT startTime, bool rollRight);
	NumberT adjustPower(NumberT amount);
	void setPower(NumberT amount);
};

