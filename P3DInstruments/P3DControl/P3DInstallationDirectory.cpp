#include "stdafx.h"
#include <sstream>
#include "P3DInstallationDirectory.h"
#include "../P3DCommon/Prepar3D.h"

std::string P3DInstallationDirectory::getInstallationDirectory(Prepar3D* p3d) {
	// Look for AppPath value under key:
	// Computer\HKEY_CURRENT_USER\Software\Lockheed Martin\Prepar3D v4
	HKEY hkeyLM;
	std::string appDir;

	LSTATUS stat = ::RegOpenKeyExA(HKEY_CURRENT_USER,
		"Software\\Lockheed Martin",
		0,
		KEY_READ,
		&hkeyLM);
	if (stat == ERROR_SUCCESS) {

		// Set up the registry sub-key by version.
		std::stringstream ss;
		ss << "Prepar3D v" << p3d->getMajorVersion();
		std::string subKey = ss.str();

		const char* value = "AppPath";
		DWORD dwType;
		char result[1024];
		DWORD size = sizeof(result);
		stat = ::RegGetValueA(hkeyLM, subKey.c_str(), value, RRF_RT_REG_SZ, &dwType, result, &size);
		if (stat != ERROR_SUCCESS) {
			::RegCloseKey(hkeyLM);
			throw FileException("Unable to get P3d installation folder(read value)", stat);
		}

		appDir = std::string(result);
	}
	else {
		throw FileException("Unable to get P3d installation folder (open registry key)", stat);
	}

	::RegCloseKey(hkeyLM);
	return appDir;
}


P3DInstallationDirectory::P3DInstallationDirectory(Prepar3D* p3d)
	: Directory(getInstallationDirectory(p3d))
{
}