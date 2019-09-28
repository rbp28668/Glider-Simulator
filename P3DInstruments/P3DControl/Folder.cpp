#include "stdafx.h"
#include <shlobj.h>
#include "WideConverter.h"

#include "Folder.h"

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "User32.lib")

// See https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
// The returned path does not include a trailing backslash. For example, "C:\Users" is returned rather than "C:\Users\".
DocumentDirectory::DocumentDirectory()
{
	PWSTR pwstr = 0;
	HRESULT result = ::SHGetKnownFolderPath(FOLDERID_Documents, 0, 0, &pwstr);

	if (result == S_OK) {
		path = ws2s(pwstr);
	}

	if (pwstr) {
		::CoTaskMemFree(pwstr);
	}
}

Directory Directory::sub(const char* name)
{
	return Directory(path + separator + name);
}

File Directory::file(const char* name)
{
	return File(path + separator + name);
}

File::ListT& Directory::files(File::ListT& fileList, const std::string& filter)
{
	WIN32_FIND_DATAA ffd;
	DWORD dwError = 0;

	std::string search = path;
	if (!filter.empty()) {
		search.append("\\");
		search.append(filter);
	}

	HANDLE hFind = FindFirstFileA(search.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		throw FileException("Unable to start file enumeration", GetLastError());
	}

	do {
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			fileList.push_back(ffd.cFileName);
		}
	} while (FindNextFileA(hFind, &ffd) != 0);
	dwError = GetLastError();
	::FindClose(hFind);

	if (dwError != ERROR_NO_MORE_FILES)	{
		throw FileException("Unable to enumerate files", dwError);
	}
	
	return fileList;
}

Directory::ListT& Directory::folders(Directory::ListT& folderList, const std::string& filter)
{
	WIN32_FIND_DATAA ffd;
	DWORD dwError = 0;

	std::string search = path;
	if (!filter.empty()) {
		search.append("\\");
		search.append(filter);
	}

	HANDLE hFind = FindFirstFileA(search.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		throw FileException("Unable to start folder enumeration", GetLastError());
	}

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			folderList.push_back(ffd.cFileName);
		}
	} while (FindNextFileA(hFind, &ffd) != 0);
	dwError = GetLastError();
	::FindClose(hFind);

	if (dwError != ERROR_NO_MORE_FILES) {
		throw FileException("Unable to enumerate folders", dwError);
	}

	return folderList;

}
