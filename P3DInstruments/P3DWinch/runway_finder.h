#pragma once

#include <SimConnect.h>
#include <string>
#include <cmath>
#include <iostream>
#include "../P3DCommon/Prepar3D.h"

struct RunwayResult {
	bool valid = false;
	double far_end_lat;   // degrees
	double far_end_lon;   // degrees
	float heading;        // degrees true
	float length;         // meters
	std::string icao;
};

// Asynchronous runway finder using SimConnect facility data API.
//
// Usage:
//   1. Call find() with the aircraft's lat/lon (radians) and heading (degrees)
//   2. Poll getState() each frame
//   3. When DONE, getResult() has the far end of the matching runway
//
// Internally performs two async steps:
//   Step 1: Request airport list, find nearest airport
//   Step 2: Request facility data for that airport, match runway by heading
class RunwayFinder : public FacilityHandler {
public:

	enum class State { IDLE, FINDING_AIRPORT, FINDING_RUNWAYS, DONE, FAILED };

	RunwayFinder(Prepar3D* p3d);

	// Start async search for the runway at the given position/heading
	void find(double lat_rad, double lon_rad, float heading_deg);

	State getState() const { return state; }
	const RunwayResult& getResult() const { return result; }

	// Called by dispatch handlers (via Prepar3D facility forwarding)
	void onAirportList(SIMCONNECT_RECV_AIRPORT_LIST* pData) override;
	void onFacilityData(SIMCONNECT_RECV_FACILITY_DATA* pData) override;
	void onFacilityDataEnd(SIMCONNECT_RECV_FACILITY_DATA_END* pData) override;

private:

	Prepar3D* p3d;
	State state = State::IDLE;
	RunwayResult result;

	// Search parameters
	double search_lat_deg;
	double search_lon_deg;
	float search_heading;
	double cos_lat;

	// Step 1: airport search
	DWORD airport_request_id = 0;
	std::string nearest_icao;
	double nearest_dist_sq;

	// Step 2: runway search
	DWORD runway_define_id = 0;
	DWORD runway_request_id = 0;
	bool definition_created = false;

	// Best runway match
	float best_heading_diff;
	double best_lat;
	double best_lon;
	float best_heading;
	float best_length;
	bool found_runway = false;

	void createDefinition();
	void requestRunwayData();
	void finalize();

	static float headingDiff(float h1, float h2);
	static constexpr double DEG_TO_RAD = 3.14159265358979 / 180.0;
	static constexpr double RAD_TO_DEG = 180.0 / 3.14159265358979;
	static constexpr double EARTH_RADIUS = 6371000.0;
};
