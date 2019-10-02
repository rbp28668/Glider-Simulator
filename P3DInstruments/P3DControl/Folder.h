#pragma once
#include <string>
#include <list>


class FileException : public std::exception {
	const char* pszReason;
	DWORD code;

public:
	FileException(const char* pszReason, DWORD code) : pszReason(pszReason), code(code) {}
	const char* what() const throw () { return pszReason; }
	DWORD err() const { return code; }
};


class File {
	std::string path;  
public:
	typedef std::list<File> ListT;

	File(const std::string& path) : path(path) {}
	File(const char* path) : path(path) {}

	const std::string& name() const { return path; }
	operator const std::string&() const { return path; }
	explicit operator const char*() const { return path.c_str(); }
	bool exists() const;
};

class Directory
{
protected:
	std::string path; // without any trailing slash.
	char separator = '\\';
	void trim(std::string& name);
public:
	typedef std::list<Directory> ListT;

	Directory() {}
	Directory(const std::string& path);
	Directory(const char* path);
	Directory sub(const char* name);
	Directory sub(const std::string& name) { return sub(name.c_str()); }
	File file(const char* name); 
	File file(const std::string& name) { return file(name.c_str()); }
	File::ListT& files( File::ListT& fileList, const std::string& filter = "" );
	Directory::ListT& folders(Directory::ListT& folderList, const std::string& filter = "");
};

// Folder that initialises itself to the current user's Documents folder
class DocumentDirectory : public Directory
{
public:
	DocumentDirectory();
};

