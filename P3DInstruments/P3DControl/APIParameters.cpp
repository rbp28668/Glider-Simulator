#include "stdafx.h"
#include "APIParameters.h"

// Expectation is that query string will be a standard html query string
// starting with ? and have name=value pairs separated by & or ;.
// It may be URL encoded (not implemented at this point).

APIParameters::APIParameters(const std::string& queryString)
{

	std::string::const_iterator it = queryString.begin(); 
	
	// skip over any ? at the start of the string
	if (it != queryString.end() && *it == '?') {
		++it;
	}
	
	int state = 0; // capturing name
	std::string name;
	std::string value;

	for (; it != queryString.end(); ++it) {
		switch (state) {
		case 0: // capturing name
			if (*it == '=') {
				state = 1; // capturing value
			}
			else {
				name.push_back(*it);
			}
			break;

		case 1: // capturing value;
			if (*it == '&' || *it == ';') { // next field?
				params[name] = value;
				name.clear();
				value.clear();
				state = 0;
			}
			else if (*it == '%') { // URL encoded value.
				char buff[3];
				++it; // skip %
				if (it == queryString.end()) break;
				buff[0] = *it; // first hex digit
				++it;
				if (it == queryString.end()) break;
				buff[1] = *it; // second hex digit
				buff[2] = 0;
				char ch = (char)std::stoi(buff,0,16);
				value.push_back(ch);
			}
			else {
				value.push_back(*it);
			}
		}
	}
	// Make sure we store last one.
	if (!name.empty() && !value.empty()) {
		params[name] = value;
	}
}

APIParameters::~APIParameters()
{
}

std::string APIParameters::getString(const std::string& name) const
{
	ParamsT::const_iterator it = params.find(name);
	if (it != params.end()) {
		return it->second;
	}
	return std::string(); // not found so empty string
}

int APIParameters::getInt(const std::string& name, int default) const
{
	ParamsT::const_iterator it = params.find(name);
	if (it != params.end()) {
		return stoi(it->second);
	}
	return default;
}

float APIParameters::getFloat(const std::string& name, float default) const
{
	ParamsT::const_iterator it = params.find(name);
	if (it != params.end()) {
		return stof(it->second);
	}

	return default;
}

bool APIParameters::getBool(const std::string& name, bool default) const
{
	ParamsT::const_iterator it = params.find(name);
	if (it != params.end()) {
		return it->second == "true" || it->second == "TRUE";
	}

	return default;
}

DWORD APIParameters::getDWORD(const std::string& name, DWORD default) const
{
	ParamsT::const_iterator it = params.find(name);
	if (it != params.end()) {
		return stoul(it->second);
	}

	return default;
}
