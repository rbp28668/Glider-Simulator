#pragma once
#include <string>
#include <map>

// Class to parse and provide parameters for the REST API
// Uses a URL format query string and converts it into a parameter map.
class APIParameters
{
	typedef std::map<std::string, std::string> ParamsT;
	ParamsT params;

public:
	APIParameters(const std::string& queryString);
	~APIParameters();

	std::string getString(const std::string& name) const;
	int getInt(const std::string& name, int default=0) const;
	float getFloat(const std::string& name, float default = 0) const;
	bool getBool(const std::string& name, bool default = false) const;
	DWORD getDWORD(const std::string& name, DWORD default = 0) const;


};

