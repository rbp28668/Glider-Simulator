#pragma once
#include <exception>
#include <string>

// Exception class for HTTP related events.
class HTTPException : public std::exception
{
	const char* pszMsg;
	unsigned long errCode;

public:
	HTTPException(const char* pszMsg, unsigned long code)
		: pszMsg(pszMsg)
		, errCode(code)
	{}

	const char* what() const throw () {
		return pszMsg;
	}

	unsigned long code() const {
		return errCode;
	}
};

