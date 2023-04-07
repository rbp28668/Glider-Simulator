#pragma once
#include "Folder.h"
#include <string>

class Prepar3D;

class P3DInstallationDirectory :
	public Directory
{
	int version;
	std::string getInstallationDirectory(Prepar3D* p3d);
public:
	P3DInstallationDirectory(Prepar3D* p3d);
}; 
