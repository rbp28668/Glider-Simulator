#include "stdafx.h"
#include <iostream>
#include "WeatherStation.h"
#include "Prepar3D.h"
#include "Metar.h"

WeatherStation::WeatherStation(Prepar3D* p3d)
	: WeatherStation(p3d,"GLOB")
{ }

WeatherStation::WeatherStation(Prepar3D* p3d, const char* szIcao)
	: icao(szIcao)
	, p3d(p3d)
	, owned(false)
	, requestId(-1)
	, updated(false)
{ }

WeatherStation::WeatherStation(Prepar3D* p3d, const char* szIcao, const char* szName, float lat, float lon)
	: icao(szIcao)
	, p3d(p3d)
	, owned(false)
	, requestId(-1)
	, updated(false)
{
	HRESULT hr = SimConnect_WeatherCreateStation(
		p3d->getHandle(),
		p3d->nextRequestId(),
		szIcao,
		szName,
		lat,
		lon,
		10.0 // feet above ground
	);
	if (hr == S_OK) {
		owned = true;
	}

}

WeatherStation::~WeatherStation()
{
	if (p3d && owned) {
		HRESULT hr = ::SimConnect_WeatherRemoveStation(
			p3d->getHandle(),
			p3d->nextRequestId(),
			icao.c_str()
		);
	}
}

void WeatherStation::requestWeather()
{
	if (requestId == -1) {
		requestId = p3d->nextRequestId();
	}

	HRESULT hr = ::SimConnect_WeatherRequestObservationAtStation(
		p3d->getHandle(),
		requestId,
		icao.c_str()
	);

}

void WeatherStation::setWeather(Metar& metar, DWORD seconds)
{
	lastWeather = metar;
	HRESULT hr = ::SimConnect_WeatherSetObservation(
		p3d->getHandle(),
		seconds,
		metar.text().c_str()
	);
}

void WeatherStation::updateWeather(Metar& metar, DWORD seconds)
{
	lastWeather.merge(metar);
	std::cout << lastWeather.text() << std::endl;

	HRESULT hr = ::SimConnect_WeatherSetObservation(
		p3d->getHandle(),
		seconds,
		lastWeather.text().c_str()
	);
}

void WeatherStation::updateFromSim(const char* pszMetar)
{
	lastWeather.parse(pszMetar);
	updated = true;
}

