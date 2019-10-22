#include "stdafx.h"
#include "Extension.h"

Extension::Extension(std::string typeCode, int start, int finish)
	:typeCode(typeCode)
	, start(start)
	, finish(finish) {
}

std::string Extension::getTypeCode() const {
	return typeCode;
}

int Extension::getStart() const {
	return start;
}

int Extension::getFinish() const {
	return finish;
}

int Extension::length() const{
	return 1 + finish - start;
}

