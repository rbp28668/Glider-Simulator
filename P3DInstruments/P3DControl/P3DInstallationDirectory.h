#pragma once
#include "Folder.h"
#include <string>


class P3DInstallationDirectory :
	public Directory
{
	std::string getInstallationDirectory();
public:
	P3DInstallationDirectory();
}; 
