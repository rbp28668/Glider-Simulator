#pragma once
#include <string>
#include "MessageHandler.h"
#include "Folder.h"
#include "../P3DCommon/Metar.h"

class Prepar3D;
class File;
class JSONWriter;
class APIParameters;

class WeatherMessageHandler :
	public MessageHandler
{
	Simulator* pSim;
	static void processTheme(File& file, JSONWriter& json);
	static void tidy(std::string& value);

	static bool processMetarParam(const APIParameters& params, Metar::FieldType type, const char* pszName, Metar& metar, std::ostringstream& errorMessage, bool& valid);
	static bool buildMetar(const APIParameters& params, Metar& metar, std::string& error);
public:

	WeatherMessageHandler(Prepar3D* p3d);
	virtual ~WeatherMessageHandler();

	static bool createStation(Simulator* pSim, const std::string& icao, const std::string& name);
	static bool createStation(Simulator* pSim, const std::string& icao, const std::string& name, float latitude, float longitude);
	static bool setGlobal(Simulator* pSim);
	static bool setCustom(Simulator* pSim);
	static bool setTheme(Simulator* pSim, const std::string& themeName);
	static void processTheme(File& file, std::string& name, std::string& title, std::string& description);
	static File::ListT& listThemes(File::ListT& fileList);
	static bool refresh(Simulator* pSim);
	static std::string requestMetar(Simulator* pSim, const std::string& icao, std::string& name);
	static std::string requestGlobalMetar(Simulator* pSim);
	static bool addStation(Simulator* pSim, const std::string& icao);
	static std::string addWeatherStationHere(Simulator* pSim, const std::string& icao, const std::string& name);
	static std::string addWeatherStationAt(Simulator* pSim, const std::string& icao, const std::string& name, float latitude, float longitude);
	static bool update(Simulator* pSim, Metar& m, DWORD seconds);
	static bool updateGlobal(Simulator* pSim, Metar& m, DWORD seconds);
	static bool set(Simulator* pSim, Metar& m, DWORD seconds);
	static bool setGlobal(Simulator* pSim, Metar& m, DWORD seconds);

	virtual void run(const std::string& cmd, const APIParameters& params, std::string& output);

private:
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
	void getWeatherStations(std::string& output);

	void update(Metar& m, DWORD seconds, std::string& output);
	void updateGlobal(Metar& m, DWORD seconds, std::string& output);

	void set(Metar& m, DWORD seconds, std::string& output);
	void setGlobal(Metar& m, DWORD seconds, std::string& output);

};

