#include <string>
#include <vector>
#pragma once

// Models an extension record in an IGC file.
class Extension {

	std::string typeCode;
	int start;  // 1 based index of first char
	int finish; // 1 based index of last char

public:
	using ListType = std::vector<Extension>;

	Extension(std::string typeCode, int start, int finish);
	std::string getTypeCode();
	int getStart();
	int getFinish();
	int length();
};

