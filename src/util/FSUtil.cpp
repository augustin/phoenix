/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "FSUtil.h"

#include "OSUtil.h"
#include "StringUtil.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include <cctype>
#include <cstdlib>

#ifndef _MSC_VER
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#  include <dirent.h>
#  include <cstring>
#else /* _MSC_VER */
#  define WIN32_LEAN_AND_MEAN
#  include <direct.h>
#  include <windows.h>
#  define PATH_MAX MAX_PATH
#endif

using std::string;
using std::vector;

// Static member variables
vector<string> FSUtil::fPATHs;

bool FSUtil::exists(const string& path)
{
	string filename = path;
#ifdef _WIN32
	StringUtil::replaceAll(filename, "/", "\\");
#endif

	std::ifstream f(filename);
	return static_cast<bool>(f);
}

bool FSUtil::isFile(const string& path)
{
	struct ::stat statbuf;
	if (::stat(path.c_str(), &statbuf) == -1)
		return false;
	if (statbuf.st_mode & S_IFREG)
		return true;
	return false;
}

bool FSUtil::isDir(const string& path)
{
	struct ::stat statbuf;
	if (::stat(path.c_str(), &statbuf) == -1)
		return false;
	if (statbuf.st_mode & S_IFDIR)
		return true;
	return false;
}

bool FSUtil::isExec(const string& path)
{
#ifndef _WIN32
	struct ::stat statbuf;
	if (::stat(path.c_str(), &statbuf) == 0) {
		return ((statbuf.st_mode & S_IFREG) && (statbuf.st_mode & 0111));
	}
	return false;
#else
	return !path.empty() && isFile(path); // Because who knows, anyway.
#endif
}

bool FSUtil::isPathAbsolute(const string& path)
{
	return (path[0] == '/'
#ifdef _WIN32
		|| (path.length() > 2 && path[1] == ':' && path[2] == '/') // "C:/" or the like
#endif
		);
}

string FSUtil::getContents(const string& file)
{
	std::ifstream filestream(file);
	// extra ()s here are supposedly mandatory?
	return string((std::istreambuf_iterator<char>(filestream)),
		std::istreambuf_iterator<char>());
}

void FSUtil::setContents(const string& file, const string& contents)
{
	std::ofstream filestream(file);
	filestream << contents;
}

void FSUtil::deleteFile(const string& file)
{
	::remove(file.c_str());
}

#ifndef _MSC_VER
void FSUtil_fileSearchHelper(vector<string>& ret, const string& dir,
	const vector<string>& exts, bool recursive)
{
	DIR* dp = ::opendir(dir.c_str());
	if (dp == nullptr) {
		// Path does not exist or could not be read
		return;
	}

	struct ::dirent* entry;
	while ((entry = ::readdir(dp)) != nullptr) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		string fullPath = FSUtil::combinePaths({dir, entry->d_name});
		if (recursive && FSUtil::isDir(fullPath))
			FSUtil_fileSearchHelper(ret, fullPath, exts, recursive);
		else {
			for (string ext : exts) {
				if (StringUtil::endsWith(fullPath, ext)) {
					ret.push_back(fullPath);
					break;
				}
			}
		}
	}

	closedir(dp);
}
#else /* _MSC_VER  */
void FSUtil_fileSearchHelper(vector<string>& ret, const string& dir,
	const vector<string>& exts, bool recursive)
{
	// Specify a file mask.
	string path = dir + "\\*";

	WIN32_FIND_DATAA file;
	HANDLE findHndl = nullptr;
	if ((findHndl = FindFirstFileA(path.c_str(), &file)) == INVALID_HANDLE_VALUE) {
		// Path not found
		return;
	}

	do {
		if (strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0)
			continue;
		// Build the path
		path = dir + "\\" + file.cFileName;

		if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			FSUtil_fileSearchHelper(ret, path, exts, recursive);
		} else {
			for (string ext : exts) {
				if (StringUtil::endsWith(path, ext)) {
					ret.push_back(FSUtil::normalizePath(path));
					break;
				}
			}
		}
	} while (FindNextFileA(findHndl, &file));

	FindClose(findHndl);
}
#endif

vector<string> FSUtil::searchForFiles(const string& dir, const vector<string>& exts,
	bool recursive)
{
	vector<string> ret;
	FSUtil_fileSearchHelper(ret, dir, exts, recursive);
	return ret;
}

string FSUtil::which(const string& prog)
{
	if (prog.empty())
		return prog;

	const string program = normalizePath(prog);
	if (isPathAbsolute(program))
		return program; // already an absolute path

#ifdef _WIN32
	string pathext = OSUtil::getEnv("PATHEXT");
	std::transform(pathext.begin(), pathext.end(), pathext.begin(), ::tolower);
	const vector<string> pathExts = StringUtil::split(pathext, ";");
	auto permutePathExt = [&](const string& path) {
		for (string ext : pathExts) {
			string fullPath = path + ext;
			if (exists(fullPath))
				return fullPath;
		}
		return string();
	};
#endif

	// The program's name contains a slash, ignore PATH
	if (program.find('/') != string::npos) {
		string normd = normalizePath(program);
		if (isExec(normd))
			return normd;
#ifdef _WIN32
		else
			return permutePathExt(normalizePath(program));
#endif
	}

	if (fPATHs.empty()) {
#ifdef _WIN32
		fPATHs = StringUtil::split(OSUtil::getEnv("PATH"), ";");
#else
		fPATHs = StringUtil::split(OSUtil::getEnv("PATH"), ":");
#endif
	}

	if (fPATHs.empty())
		return "";
	for (string path : fPATHs) {
		string fullPath =
#ifdef _WIN32
			permutePathExt(combinePaths({path, program}));
#else
			combinePaths({path, program});
#endif
		if (isExec(fullPath))
			return fullPath;
	}
	return "";
}

string FSUtil::normalizePath(const string& path)
{
	if (path.empty())
		return path;

	string ret = path;
#ifdef _WIN32
	StringUtil::replaceAll(ret, "\\", "/");
#endif
	vector<string> orig = StringUtil::split(ret, "/"), normd;
	for (string i : orig) {
		if (i == ".")
			continue;
		else if (i.empty() && normd.size() > 0) // preserve the opening '/'
			continue;
		else if (i == ".." && normd.size() > 0 && normd[normd.size() - 1] != "..")
			normd.pop_back();
		else
			normd.push_back(i);
	}

	// Now that we've normalized the vector containing the individual components,
	// rebuild the actual path string.
	ret = "";
	for (vector<string>::size_type i = 0; i < normd.size(); i++) {
		if (i != 0)
			ret += "/";
		ret += normd[i];
	}

#ifdef _WIN32
	// Turn Cygwin/MSYS-style paths (e.g. '/c/Windows/') into drive+path format
	// (e.g. "C:/Windows/").
	if (ret.length() > 2 && ret[0] == '/' && ret[2] == '/') {
		ret[0] = ::toupper(ret[1]);
		ret[1] = ':';
	}
#endif
	return ret;
}

string FSUtil::combinePaths(const vector<string>& paths)
{
	string ret;
	for (string path : paths) {
		bool endsInSeparator =
			ret.length() == 0 ||
			StringUtil::endsWith(ret, "/")
#ifdef _WIN32
			|| StringUtil::endsWith(ret, "\\")
#endif
			;

		if (!endsInSeparator) {
			ret += "/";
		}
		ret += path;
	}
	return normalizePath(ret);
}

string FSUtil::absolutePath(const string& path)
{
	string ret = normalizePath(path);
	if (path[0] == '/'
#ifdef _WIN32
		|| (path.size() > 2 && path[1] == ':' && path[2] == '/')
#endif
		)
		return ret;
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		ret = FSUtil::combinePaths({cwd, path});
	return ret;
}

string FSUtil::parentDirectory(const string& path)
{
	string ret = normalizePath(path);
	string::size_type pos = ret.rfind("/", ret.length() - 1 /* in case it ends with a '/' */);
	if (pos == string::npos)
		return "."; // No '/'s in the path; so it's just a single file.
	return ret.substr(0, pos);
}

void FSUtil::mkdir(const string& path)
{
#ifdef _MSC_VER
	::_mkdir(path.c_str());
#else
	::mkdir(path.c_str()
#ifndef _WIN32
		, 0755 // mode
#endif
		);
#endif
}
void FSUtil::rmdir(const string& path)
{
	::rmdir(path.c_str());
}
