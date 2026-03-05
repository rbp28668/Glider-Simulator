
#include "runway_finder.h"
#include "../P3DCommon/Prepar3D.h"


RunwayFinder::RunwayFinder(Prepar3D* p3d)
	: p3d(p3d)
{
}


void RunwayFinder::find(double lat_rad, double lon_rad, float heading_deg) {
	search_lat_deg = lat_rad * RAD_TO_DEG;
	search_lon_deg = lon_rad * RAD_TO_DEG;
	search_heading = heading_deg;
	cos_lat = cos(lat_rad);

	nearest_dist_sq = 1e30;
	nearest_icao = "";
	found_runway = false;
	result.valid = false;

	state = State::FINDING_AIRPORT;
	airport_request_id = p3d->nextRequestId();

	HANDLE h = p3d->getHandle();
	HRESULT hr = SimConnect_RequestFacilitiesList(h, SIMCONNECT_FACILITY_LIST_TYPE_AIRPORT, airport_request_id);
	if (FAILED(hr)) {
		std::cerr << "Failed to request airport list" << std::endl;
		state = State::FAILED;
	}
}


// ---------------------------------------------------------------------------
//  Step 1: Receive airport list packets, track nearest airport
// ---------------------------------------------------------------------------

void RunwayFinder::onAirportList(SIMCONNECT_RECV_AIRPORT_LIST* pData) {
	if (state != State::FINDING_AIRPORT) return;
	if (pData->dwRequestID != airport_request_id) return;

	for (DWORD i = 0; i < pData->dwArraySize; i++) {
		auto& apt = pData->rgData[i];
		double dlat = apt.Latitude - search_lat_deg;
		double dlon = (apt.Longitude - search_lon_deg) * cos_lat;
		double dist_sq = dlat * dlat + dlon * dlon;

		if (dist_sq < nearest_dist_sq) {
			nearest_dist_sq = dist_sq;
			nearest_icao = apt.Icao;
		}
	}

	// Check if we've received all airports
	DWORD received = pData->dwEntryNumber + pData->dwArraySize;
	if (received >= pData->dwOutOf) {
		if (nearest_icao.empty()) {
			std::cerr << "No airports found" << std::endl;
			state = State::FAILED;
		}
		else {
			std::cout << "Nearest airport: " << nearest_icao << std::endl;
			requestRunwayData();
		}
	}
}


// ---------------------------------------------------------------------------
//  Step 2: Request runway data for the nearest airport
// ---------------------------------------------------------------------------

void RunwayFinder::createDefinition() {
	if (definition_created) return;

	runway_define_id = p3d->nextRequestId();
	HANDLE h = p3d->getHandle();

	SimConnect_AddToFacilityDefinition(h, runway_define_id, "OPEN AIRPORT");
	SimConnect_AddToFacilityDefinition(h, runway_define_id, "OPEN RUNWAY");
	SimConnect_AddToFacilityDefinition(h, runway_define_id, "HEADING");
	SimConnect_AddToFacilityDefinition(h, runway_define_id, "LENGTH");
	SimConnect_AddToFacilityDefinition(h, runway_define_id, "LATITUDE");
	SimConnect_AddToFacilityDefinition(h, runway_define_id, "LONGITUDE");
	SimConnect_AddToFacilityDefinition(h, runway_define_id, "CLOSE RUNWAY");
	SimConnect_AddToFacilityDefinition(h, runway_define_id, "CLOSE AIRPORT");

	definition_created = true;
}


void RunwayFinder::requestRunwayData() {
	createDefinition();

	state = State::FINDING_RUNWAYS;
	found_runway = false;
	best_heading_diff = 180.0f;

	runway_request_id = p3d->nextRequestId();
	HANDLE h = p3d->getHandle();

	HRESULT hr = SimConnect_RequestFacilityData(h, runway_define_id, runway_request_id, nearest_icao.c_str());
	if (FAILED(hr)) {
		std::cerr << "Failed to request runway data for " << nearest_icao << std::endl;
		state = State::FAILED;
	}
}


// ---------------------------------------------------------------------------
//  Step 2 response: Receive facility data for each runway
// ---------------------------------------------------------------------------

void RunwayFinder::onFacilityData(SIMCONNECT_RECV_FACILITY_DATA* pData) {
	if (state != State::FINDING_RUNWAYS) return;
	if (pData->dwRequestID != runway_request_id) return;

	// Only process runway records (type 1 = SIMCONNECT_FACILITY_DATA_RUNWAY)
	if (pData->dwType != 1) return;

	// Data layout matches our definition: HEADING(float), LENGTH(float), LATITUDE(double), LONGITUDE(double)
#pragma pack(push, 1)
	struct RunwayFields {
		float heading;
		float length;
		double latitude;
		double longitude;
	};
#pragma pack(pop)

	auto* rwy = reinterpret_cast<const RunwayFields*>(&pData->dwData);

	// Check both runway directions (primary heading and reciprocal)
	float diff_primary = headingDiff(search_heading, rwy->heading);
	float diff_recip = headingDiff(search_heading, fmodf(rwy->heading + 180.0f, 360.0f));

	// The matched heading is the one closest to our aircraft heading.
	// The winch goes at the FAR end: in the direction we're heading.
	float diff;
	float matched_heading;
	if (diff_primary <= diff_recip) {
		diff = diff_primary;
		matched_heading = rwy->heading;  // aircraft heading matches primary
	}
	else {
		diff = diff_recip;
		matched_heading = fmodf(rwy->heading + 180.0f, 360.0f);  // aircraft matches reciprocal
	}

	if (diff < best_heading_diff) {
		best_heading_diff = diff;
		best_lat = rwy->latitude;
		best_lon = rwy->longitude;
		best_heading = matched_heading;
		best_length = rwy->length;
		found_runway = true;
	}
}


void RunwayFinder::onFacilityDataEnd(SIMCONNECT_RECV_FACILITY_DATA_END* pData) {
	if (state != State::FINDING_RUNWAYS) return;
	if (pData->dwRequestID != runway_request_id) return;

	if (!found_runway || best_heading_diff > 30.0f) {
		std::cerr << "No matching runway found (best heading diff: " << best_heading_diff << " deg)" << std::endl;
		state = State::FAILED;
		return;
	}

	finalize();
}


// ---------------------------------------------------------------------------
//  Calculate the far end of the matched runway
// ---------------------------------------------------------------------------

void RunwayFinder::finalize() {
	// Runway center is at (best_lat, best_lon).
	// The far end (winch position) is half the length in the matched heading direction.
	double heading_rad = best_heading * DEG_TO_RAD;
	double half_length = best_length / 2.0;

	double dlat_m = half_length * cos(heading_rad);
	double dlon_m = half_length * sin(heading_rad);

	double metres_per_deg_lat = EARTH_RADIUS * DEG_TO_RAD;
	double metres_per_deg_lon = EARTH_RADIUS * DEG_TO_RAD * cos(best_lat * DEG_TO_RAD);

	result.valid = true;
	result.far_end_lat = best_lat + dlat_m / metres_per_deg_lat;
	result.far_end_lon = best_lon + dlon_m / metres_per_deg_lon;
	result.heading = best_heading;
	result.length = best_length;
	result.icao = nearest_icao;

	state = State::DONE;

	std::cout << "Found runway heading " << (int)best_heading
		<< " at " << nearest_icao
		<< ", length " << (int)best_length << "m" << std::endl;
}


float RunwayFinder::headingDiff(float h1, float h2) {
	float diff = fmodf(fabsf(h1 - h2), 360.0f);
	return diff > 180.0f ? 360.0f - diff : diff;
}
