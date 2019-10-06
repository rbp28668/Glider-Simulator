#include "stdafx.h"
#include <shlobj.h>
#include <Shlwapi.h>
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

// Trim and leading or trailing separators.
void Directory::trim(std::string& name)
{
	while (name.front() == separator) name = name.substr(1);
	while (name.back() == separator) name.pop_back();
}

Directory::Directory(const std::string& path)
	: path(path)
{
	trim(this->path);  // note windows path assumed X:/root/sub etc so no leading /
}

Directory::Directory(const char* path)
	: Directory(std::string(path))
{
}

Directory Directory::sub(const char* name)
{
	std::string trimmed(name);
	trim(trimmed);

	std::string newPath = path + separator + trimmed;
	return Directory(newPath);
}

File Directory::file(const char* name)
{
	std::string trimmed(name);
	trim(trimmed);
	return File(path + separator + trimmed);
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
			fileList.push_back(file(ffd.cFileName));
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
	search.append("\\"); // as want to find sub-folders.

	if (filter.empty()) {
		search.append("*");
	} 
	else {
		search.append(filter);
	}

	HANDLE hFind = ::FindFirstFileA(search.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		throw FileException("Unable to start folder enumeration", GetLastError());
	}

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (ffd.cFileName[0] != '.') {  // ignore both . and .. folders
				folderList.push_back(this->sub(ffd.cFileName));
			}
		}
	} while (FindNextFileA(hFind, &ffd) != 0);
	dwError = GetLastError();
	::FindClose(hFind);

	if (dwError != ERROR_NO_MORE_FILES) {
		throw FileException("Unable to enumerate folders", dwError);
	}

	return folderList;

}

const std::string File::name() const
{
	size_t pos = path.find_last_of('\\');
	if (pos != std::string::npos) {
		return path.substr(pos+1);
	}
	else {
		return path;
	}
}

bool File::exists() const
{
	return ::PathFileExistsA(path.c_str()) == TRUE;
}
