#pragma once
#include "WeatherStation.h"
#include <string>
#include <map>

class Prepar3D;

class WeatherStations
{
	Prepar3D* p3d;
	WeatherStation* globalWeatherStation;
	int sequence;
	using StationMap = std::map<std::string, WeatherStation*>;
	StationMap stations;

public:
	WeatherStations(Prepar3D* p3d);
	~WeatherStations();

	WeatherStation* globalWeather() { return globalWeatherStation; }
	void update(const char* pszMetar);
	void add(const char* szIACO);
	void add(const char* szIcao, const char* szName, float lat, float lon);
	std::string add(float lat, float lon);
	void refresh();
	WeatherStation* get(const std::string& ICAO);

};

