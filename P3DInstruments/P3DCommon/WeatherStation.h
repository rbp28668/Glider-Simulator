#pragma once
#include <string>
#include "Metar.h"
class Prepar3D;

class WeatherStation
{
	Prepar3D* p3d;
	std::string icao;
	bool owned; // true if we've created this one, false if existing.
	LONG requestId;  // used for p3D requests.
	Metar lastWeather; // set up by requestWeather.

public:
	WeatherStation(Prepar3D* p3d);
	WeatherStation(Prepar3D* p3d,const char* szIcao);
	WeatherStation(Prepar3D* p3d, const char* szIcao, const char* szName, float lat, float lon);
	~WeatherStation();

	const std::string& name() const { return icao; }
	void requestWeather();
	Metar& lastWeatherReport() { return lastWeather; }
	void updateWeather(Metar& metar, DWORD seconds);
	void setWeather(Metar& metar, DWORD seconds=0);
	void updateFromSim(const char* pszMetar);
};

