#include "stdafx.h"
#include "Extension.h"

Extension::Extension(std::string typeCode, int start, int finish)
	:typeCode(typeCode)
	, start(start)
	, finish(finish) {
}

std::string Extension::getTypeCode() {
	return typeCode;
}

int Extension::getStart() {
	return start;
}

int Extension::getFinish() {
	return finish;
}

int Extension::length() {
	return 1 + finish - start;
}

