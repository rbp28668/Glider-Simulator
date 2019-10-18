#include "stdafx.h"

#include "WideConverter.h"

std::string ws2s(const std::wstring& s)
{
	int slength = (int)s.length();

	// How many chars needed?
	int len = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, 0, 0, 0, 0);

	std::string r(len, '\0'); // empty string of just the right size

	// Transform into r.
	::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, &r[0], len, 0, 0);
	return r;
}
