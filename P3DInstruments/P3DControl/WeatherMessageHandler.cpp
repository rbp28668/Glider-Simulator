#include "stdafx.h"
#include <fstream>
#include <sstream>
#include "Simulator.h"
#include "WeatherMessageHandler.h"
#include "SimState.h"
#include "P3DInstallationDirectory.h"
#include "JSONWriter.h"
#include "../P3DCommon/WeatherStations.h"
#include "WideConverter.h"
#include "APIParameters.h"

WeatherMessageHandler::WeatherMessageHandler(Prepar3D* p3d)
	: pSim ( static_cast<Simulator*>(p3d))
	, MessageHandler(p3d,"weather")
{
}

WeatherMessageHandler::~WeatherMessageHandler()
{
}

void WeatherMessageHandler::run(const std::string& cmd, const APIParameters& params, std::string& output)
{
	// List all available themes
	if (cmd == "themes") {
		listWeatherThemes(output);
	}
	// Select a theme
	else if (cmd == "theme") {
		std::string theme = params.getString("name");
		if (theme.empty()) {
			reportFailure("Must provide a theme name to set", 0, output);
		}
		else {
			setWeatherTheme(theme, output);
		}
	}
	// Set global weather mode
	else if (cmd == "global") {
		setGlobalWeather(output);
	}
	// Set custom weather mode
	else if (cmd == "custom") {
		setCustomWeather(output);
	}
	// Request metar for a given station
	else if (cmd == "request") {
		std::string icao = params.getString("name");
		if (icao.empty()) {
			reportFailure("Must give icao code to request weather", 0, output);
		}
		else {
			requestWeather(icao, output);
		}
	}
	// request metar for the global weather.
	else if (cmd == "request_global") { 
		requestGlobalWeather(output);
	}
	// refresh all the registered weather stations
	else if (cmd == "refresh") {
		refresh(output);
	}
	// Add an existing weather station to the ones we're interested in
	else if (cmd == "add") {
		std::string icao = params.getString("icao");
		if (icao.empty()) {
			reportFailure("Must give icao code to add weather station", 0, output);
		}
		else {
			addWeatherStation(icao, output);
		}
	}
	// adds a new weather station to the current position
	else if (cmd == "add_here") {
		std::string icao = params.getString("icao");
		std::string name = params.getString("name");
		addWeatherStationHere(icao, name, output);
	}
	// Updates the weather at the given weather station.  Partial updates
	// are merged with the current state.
	else if (cmd == "update") {
		DWORD seconds = params.getDWORD("seconds");
		Metar m;
		std::string error("Unable to update weather station");
		if (buildMetar(params, m, error)) {
			update(m, seconds, output);
		}
		else {
			reportFailure(error.c_str(), 0, output);
		}
	}
	// Updates the global weather.  Partial updates are merged.
	else if (cmd == "update_global") {
		DWORD seconds = params.getDWORD("seconds");
		Metar m;
		std::string error("Unable to update global weather");
		if (buildMetar(params, m, error)) {
			m.set(Metar::STATION, "GLOB");
			updateGlobal(m, seconds, output);
		}
		else {
			reportFailure(error.c_str(), 0, output);
		}
	}
	// Sets the weather at the given station completely over-writing
	// any existing data
	else if (cmd == "set") {
		DWORD seconds = params.getDWORD("seconds");
		Metar m;
		std::string error("Unable to set weather station");
		if (buildMetar(params, m, error)) {
			set(m, seconds, output);
		}
		else {
			reportFailure(error.c_str(), 0, output);
		}
	}
	// Sets the global weather completely over-writing any existing data.
	else if (cmd == "set_global") {
		DWORD seconds = params.getDWORD("seconds");
		Metar m;
		std::string error("Unable to update global weather");
		if (buildMetar(params, m, error)) {
			m.set(Metar::STATION, "GLOB");
			setGlobal(m, seconds, output);
		}
		else {
			reportFailure(error.c_str(), 0, output);
		}
	}
	// Sets the weather at the given station from a single metar string completely over-writing
	// any existing data
	else if (cmd == "set_metar") {
		std::string metarText = params.getString("metar");
		DWORD seconds = params.getDWORD("seconds");
		Metar m;
		std::string residual = m.parse(metarText);
		if (residual.empty()) {
			set(m, seconds, output);
		}
		else { // wasn't able to parse whole string so fail...
			JSONWriter json(output);
			json.add("status", "FAILED");
			json.add("reason", "Unable to parse complete metar");
			json.add("residual", residual);
			json.add("metar", m.show());
		}
	}
	// Sets the global weather from a single metar string completely over-writing any existing data.
	else if (cmd == "set_global_metar") {
		std::string metarText = params.getString("metar");
		DWORD seconds = params.getDWORD("seconds");
		Metar m;
		std::string residual = m.parse(metarText);
		if (residual.empty()) {
			m.set(Metar::STATION, "GLOB");
			setGlobal(m, seconds, output);
		}
		else { // wasn't able to parse whole string so fail...
			JSONWriter json(output);
			json.add("status", "FAILED");
			json.add("reason", "Unable to parse complete metar");
			json.add("residual", residual);
			json.add("metar", m.show());
		}
	}

	else {
		reportFailure("Unknown weather command", 0, output);
	}
}

// Creates a weather station at the given location.
void WeatherMessageHandler::createStationHere(const std::string& icao, const std::string& name, std::string& output)
{
	SimState::Data currentPosition = pSim->getState()->current();
	pSim->weatherStations().add(icao.c_str(), name.c_str(), (float) currentPosition.Latitude, (float) currentPosition.Longitude);
	reportSuccess(output);
}

// Enables global weather
void WeatherMessageHandler::setGlobalWeather(std::string& output)
{
	HRESULT hr = ::SimConnect_WeatherSetModeGlobal(pSim->getHandle());
	if (hr == S_OK) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to set global weather", hr, output);
	}

}

// Enables custom weather
void WeatherMessageHandler::setCustomWeather(std::string& output)
{
	HRESULT hr = ::SimConnect_WeatherSetModeCustom( pSim->getHandle() );
	if (hr == S_OK) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to set custom weather", hr, output);
	}
}

// Sets a standard weather theme
void WeatherMessageHandler::setWeatherTheme(const std::string& themeName, std::string& output)
{
	HRESULT hr = ::SimConnect_WeatherSetModeTheme(
		pSim->getHandle(),
		themeName.c_str()
	);
	if (hr == S_OK) {
		reportSuccess(output);
	}
	else {
		reportFailure("Unable to set weather theme", hr, output);
	}

}

/*
[Main]
Title = "Major Thunderstorm"
Description = "A large area of low pressure has generated a massive storm front with concentrated thunderstorms at its center."
Version = 9.00
*/
void WeatherMessageHandler::processTheme(File& file, JSONWriter& json) {
	
	std::ifstream ifs(file);

	std::string name = file.name();
	std::string title;
	std::string description;

	char ch0, ch1;
	ifs >> ch0 >> ch1;

	//std::cout << (int)ch0 << "," << (int)ch1 << std::endl;

	if (-1 == (int)ch0 && -2 == (int)ch1) { // UTF-16
		std::wstring line;
		while (ifs.good()) {

			ch0 = ifs.get();  // LS byte
			ch1 = ifs.get();  // MS byte
			wchar_t ch = (wchar_t)((unsigned int)ch1 * 256 + (unsigned int)ch0);

			if (ch == '\r') {
				// IGNORE
			}
			else if (ch == '\n') {

				if (line.find_first_of(L"Title") == 0) {
					title = ws2s(line.substr(wcslen(L"Title")));
				}
				else if (line.find_first_of(L"Description") == 0) {
					description = ws2s(line.substr(wcslen(L"Description")));
				}

				line.erase(); // start a new line
			}
			else {
				line.push_back(ch);
			}
		}
	}
	else { // Hope just ASCII!
		std::string line;
		while (ifs.good()) {

			char ch = ifs.get();

			if (ch == '\r') {
				// IGNORE
			}
			else if (ch == '\n') {

				if (line.find_first_of("Title") == 0) {
					title = line.substr(strlen("Title"));
				}
				else if (line.find_first_of("Description") == 0) {
					description = line.substr(strlen("Description"));
				}

				line.erase(); // start a new line
			}
			else {
				line.push_back(ch);
			}
		}

	}

	// tidy up name, title and description to get rid of spurious characters.
	size_t pos = name.find(".wt");
	if (pos != std::string::npos) {
		name.resize(pos);
	}
	tidy(title);
	tidy(description);

	json.object();
	json.add("name", name);
	json.add("title", title);
	json.add("description", description);
	json.end();
}

// The values in the config file will have spaces/quotes/equals sign surrounding them
// so strip these off.
void WeatherMessageHandler::tidy(std::string& value)
{
	size_t pos = 0;
	std::string strip(" =\""); // strip any of these characters off the front and rear
	while (pos < value.length()) {
		char c = value[pos];
		if (strip.find(c) == std::string::npos) {
			break; // not a char to strip so stop looking
		}
		else {
			++pos;
		}
	}
	value = value.substr(pos);
	while (!value.empty()) {
		if (strip.find(value.back()) == std::string::npos) {
			break;
		}
		else {
			value.pop_back();
		}
	}
}

// Lists all the weather themes in the default location under
// the P3D install directory.
void WeatherMessageHandler::listWeatherThemes(std::string& output)
{
	try {
		P3DInstallationDirectory p3dInstall;
		Directory themesFolder = p3dInstall.sub("Weather").sub("themes");

		File::ListT files;
		themesFolder.files(files,"*.wt");

		JSONWriter json(output);
		json.add("status", "OK");
		json.array("themes");

		for (File::ListT::iterator it = files.begin();
			it != files.end();
			++it) {
			processTheme(*it, json);
		}
		json.end(); // of array
	}
	catch (FileException& fx) {
		output.erase();
		reportFailure(fx.what(), fx.err(), output);
	}

}

// Refresh the information held by all the weather stations we're monitoring
void WeatherMessageHandler::refresh(std::string& output)
{
	pSim->weatherStations().refresh();
	reportSuccess(output);
}

// Request weather for a given weather station.
void WeatherMessageHandler::requestWeather(const std::string& icao, std::string& output)
{
	WeatherStation* station = pSim->weatherStations().get(icao);
	if (station) {

		const Metar& metar = station->lastWeatherReport();
		JSONWriter json(output);
		json.add("status", "OK");
		json.add("station", station->name());
		json.add("metar", metar.text());
	}
	else {
		reportFailure("Unknown weather station", 0, output);
	}
}

// Convenince function for getting global weather.
void WeatherMessageHandler::requestGlobalWeather(std::string& output)
{
	WeatherStation* station = pSim->weatherStations().globalWeather();
	const Metar& metar = station->lastWeatherReport();
	JSONWriter json(output);
	json.add("status", "OK");
	json.add("station", station->name());
	json.add("metar", metar.text());
}

// Add an existing weather station to the list we're interested in
void WeatherMessageHandler::addWeatherStation(const std::string& icao, std::string& output)
{
	pSim->weatherStations().add(icao.c_str());
	reportSuccess(output);
}

// Add a weather station here with the given names.  If icao/name are blank one will be created.
void WeatherMessageHandler::addWeatherStationHere(const std::string& icao, const std::string& name, std::string& output)
{
	std::string result;
	SimState::Data current = pSim->getState()->current();
	if (icao.empty() && name.empty()) {
		result = pSim->weatherStations().add((float)current.Latitude, (float)current.Longitude);
	}
	else {
		std::string n = name;
		if (n.empty()) {
			n = icao;
		}
		pSim->weatherStations().add(icao.c_str(), n.c_str(), (float)current.Latitude, (float)current.Longitude);
		result = icao;
	}
	JSONWriter json(output);
	json.add("status", "OK");
	json.add("icao", result);
}

// Processes the parameter(s) for a single metar field.
// params holds the params.
// type is the type of field.
// pszName is the name of the parameter
// metar is the metar we're trying to fill in.
// errorMessage is used to create any error messages
// valid is set if there is a failure
// returns valid.
bool WeatherMessageHandler::processMetarParam(const APIParameters& params, Metar::FieldType type, const char* pszName,  Metar& metar, std::ostringstream& errorMessage, bool& valid) {

	std::string value = params.getString(pszName);
	if (!metar.set(type, value)) {
		errorMessage << (valid ? "Metar error " : ", ");
		errorMessage << "Invalid " << metar.typeName(type) << " : " << value;
		valid = false;
	}

	// So if we read in one good non empty value and there may be multiple ones, go look...
	else if(!value.empty() && metar.multiple(type)) {
		for (int idx = 0; valid; ++idx) {
			std::ostringstream os;
			os << pszName << idx;
			value = params.getString(os.str());
			if (value.empty()) {
				break;
			}
			else {
				if (!metar.add(type, value)) {
					errorMessage << (valid ? "Metar error " : ", ");
					errorMessage << "Invalid " << metar.typeName(type) << " : " << value;
					valid = false;
				}

			}
		}
	}

	return valid;
}
// Builds a metar out of its different parameters.
bool WeatherMessageHandler::buildMetar(const APIParameters& params, Metar& metar, std::string& error) {

	std::ostringstream errorMessage;

	bool valid = true;
	if (!error.empty()) {
		errorMessage << error << ": ";
	}

	processMetarParam(params, Metar::STATION, "icao", metar, errorMessage, valid);
	processMetarParam(params, Metar::REPORT_TYPE, "report_type", metar, errorMessage, valid);
	processMetarParam(params, Metar::AUTO, "automated", metar, errorMessage, valid);
	processMetarParam(params, Metar::COR, "cor", metar, errorMessage, valid);
	processMetarParam(params, Metar::DATETIME, "datetime", metar, errorMessage, valid);
	processMetarParam(params, Metar::NIL, "nil", metar, errorMessage, valid);
	processMetarParam(params, Metar::SURFACE_WIND, "surface_wind", metar, errorMessage, valid);
	processMetarParam(params, Metar::MIN_MAX_WIND_DIR, "min_max_wind_dir", metar, errorMessage, valid);
	processMetarParam(params, Metar::WINDS_ALOFT, "winds_aloft", metar, errorMessage, valid);
	processMetarParam(params, Metar::MIN_MAX_WIND_DIR_ALOFT, "min_max_wind_dir_aloft", metar, errorMessage, valid);
	processMetarParam(params, Metar::CAVOK, "cavok", metar, errorMessage, valid);
	processMetarParam(params, Metar::VISIBILITY, "visibility", metar, errorMessage, valid);
	processMetarParam(params, Metar::RVR, "rvr", metar, errorMessage, valid);
	processMetarParam(params, Metar::PRESENT_CONDITIONS, "present_conditions", metar, errorMessage, valid);
	processMetarParam(params, Metar::PARTIAL_OBSCURATION, "partial_obscuration", metar, errorMessage, valid);
	processMetarParam(params, Metar::SKY_CONDITIONS, "sky_conditions", metar, errorMessage, valid);
	processMetarParam(params, Metar::TEMPERATURE, "temperature", metar, errorMessage, valid);
	processMetarParam(params, Metar::ALTIMETER, "altimeter", metar, errorMessage, valid);
	

	if (!valid) {
		error = errorMessage.str();
	}
	return valid;
}

// Updates the weather station identified by its icao code.
void WeatherMessageHandler::update(const Metar& m, DWORD seconds, std::string& output)
{
	std::string icao = m.get(Metar::STATION);
	if (icao.length() > 4) {
		icao = icao.substr(0, 4);
	}
	
	WeatherStation* station = pSim->weatherStations().get(icao);
	if (station) {
		station->updateWeather(m, seconds);
		reportSuccess(output);
	}
	else {
		reportFailure("Unknown weather station when updating weather", 0, output);
	}
}

// Update the global weather.
void WeatherMessageHandler::updateGlobal(const Metar& m, DWORD seconds, std::string& output)
{
	pSim->weatherStations().globalWeather()->updateWeather(m, seconds);
	reportSuccess(output);
}

// Sets the weather station identified by its icao code.
void WeatherMessageHandler::set(const Metar& m, DWORD seconds, std::string& output)
{
	std::string icao = m.get(Metar::STATION);
	if (icao.length() > 4) {
		icao = icao.substr(0, 4);
	}

	WeatherStation* station = pSim->weatherStations().get(icao);
	if (station) {
		station->setWeather(m, seconds);
		reportSuccess(output);
	}
	else {
		reportFailure("Unknown weather station when setting weather", 0, output);
	}
}

// Sets the global weather.
void WeatherMessageHandler::setGlobal(const Metar& m, DWORD seconds, std::string& output)
{
	pSim->weatherStations().globalWeather()->setWeather(m, seconds);
	reportSuccess(output);
}

