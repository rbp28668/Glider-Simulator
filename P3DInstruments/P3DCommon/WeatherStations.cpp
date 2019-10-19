#include "stdafx.h"
#include <sstream>
#include "WeatherStations.h"

WeatherStations::WeatherStations(Prepar3D* p3d)
	: p3d(p3d)
	, sequence(0)
	, globalWeatherStation(new WeatherStation(p3d))
{
	// make sure we can find and eventually delete the global weather station
	stations[globalWeatherStation->name()] = globalWeatherStation;
}

WeatherStations::~WeatherStations()
{
	for (StationMap::iterator iter = stations.begin();
		iter != stations.end();
		++iter) {
		delete iter->second;
		iter->second = 0;
	}
}

void WeatherStations::update(const char* pszMetar)
{
	std::string metar(pszMetar);
	std::string station = metar.substr(0, 4); // first 4 chars should have the ICAO code or GLOB
	StationMap::iterator it = stations.find(station);
	if (it != stations.end()) {
		it->second->updateFromSim(pszMetar);
	}
}

// Add an existing weather station to the list.
void WeatherStations::add(const char* szIACO)
{
	WeatherStation* wxs = new WeatherStation(p3d, szIACO);
	stations[wxs->name()] = wxs;
}

// Add a new weather station at the given location
void WeatherStations::add(const char* szIcao, const char* szName, float lat, float lon)
{
	WeatherStation* wxs = new WeatherStation(p3d, szIcao, szName, lat, lon);
	stations[wxs->name()] = wxs;
}

// Add a new weather station at the given location but make up a name/icao code.
std::string WeatherStations::add(float lat, float lon)
{
	// Magic up a dummy ICAO identifier.
	std::ostringstream ss;
	ss << "X";
	ss.width(3);
	ss.fill('0');
	ss << sequence;
	++sequence;

	add(ss.str().c_str(), ss.str().c_str(), lat, lon);

	return ss.str();
}

void WeatherStations::refresh()
{
	for (StationMap::iterator iter = stations.begin();
		iter != stations.end();
		++iter) {
		iter->second->requestWeather();
	}

}

WeatherStation* WeatherStations::get(const std::string& ICAO)
{
	WeatherStation* station = 0;
	StationMap::iterator iter = stations.find(ICAO);
	if (iter != stations.end()) {
		station = iter->second;
	}
	return station;
}
