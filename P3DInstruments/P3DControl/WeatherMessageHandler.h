#pragma once
#include <string>
#include "MessageHandler.h"

class Prepar3D;
class File;
class JSONWriter;
class APIParameters;

class WeatherMessageHandler :
	public MessageHandler
{
	Simulator* pSim;
	void processTheme(File& file, JSONWriter& json);
	void tidy(std::string& value);

	bool processMetarParam(const APIParameters& params, Metar::FieldType type, const char* pszName, Metar& metar, std::ostringstream& errorMessage, bool& valid);
	bool buildMetar(const APIParameters& params, Metar& metar, std::string& error);
public:
	WeatherMessageHandler(Prepar3D* p3d);
	virtual ~WeatherMessageHandler();
	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

	void createStationHere(const std::string& icao, const std::string& name, std::string& output);
	void setGlobalWeather(std::string& output);
	void setCustomWeather(std::string& output);

	void setWeatherTheme(const std::string& themeName, std::string& output);
	void listWeatherThemes(std::string& output);
	void refresh(std::string& output);

	void requestWeather(const std::string& icao, std::string& output);
	void requestGlobalWeather(std::string& output);

	void addWeatherStation(const std::string& icao, std::string& output);
	void addWeatherStationHere(const std::string& icao, const std::string& name, std::string& output);

	void update(Metar& m, DWORD seconds, std::string& output);
	void updateGlobal(Metar& m, DWORD seconds, std::string& output);

	void set(Metar& m, DWORD seconds, std::string& output);
	void setGlobal(Metar& m, DWORD seconds, std::string& output);

};

