/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "FSUtil.h"

#include "StringUtil.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include <ctype.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#else /* _MSC_VER */
#include <direct.h>
#endif

using std::string;
using std::vector;

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
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) == -1)
		return false;
	if (statbuf.st_mode & S_IFREG)
		return true;
	return false;
}

bool FSUtil::isDir(const string& path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) == -1)
		return false;
	if (statbuf.st_mode & S_IFDIR)
		return true;
	return false;
}

std::string FSUtil::getContents(const string& file)
{
	std::ifstream filestream(file);
	// extra ()s here are mandatory
	return string((std::istreambuf_iterator<char>(filestream)),
		std::istreambuf_iterator<char>());
}

void FSUtil::putContents(const std::string& file, const std::string& contents)
{
	std::ofstream filestream(file);
	filestream << contents;
}

string FSUtil::which(const string& program)
{
	if (program[0] == '/'
#ifdef _WIN32
		|| (program[0] == '\\' || (program.length() > 2 && program[1] == ':' && program[2] == '\\'))
#endif
		)
		return program; // already an absolute path
	if (program.empty())
		return program;

#ifdef _WIN32
	auto permutePathExt = [](const string& path) {
		vector<string> PathExts = StringUtil::split(getenv("PATHEXT"), ";");
		for (string ext : PathExts) {
			string fullPath = path + ext;
			if (exists(fullPath))
				return fullPath;
		}
		return string();
	};
#endif

	// The program's name contains a slash, ignore PATH
	if (program.find('/') != string::npos
#ifdef _WIN32
		|| program.find('\\') != string::npos
#endif
		) {
		string normd = normalizePath(program);
		if (exists(normd)) {
			// TODO: check if 'program' is executable
			// TODO: return absolute path
			return normd;
		}
#ifdef _WIN32
		else
			return permutePathExt(normalizePath(program));
#endif
		return "";
	}

	vector<string> PATHs =
#ifdef _WIN32
		StringUtil::split(getenv("PATH"), ";");
#else
		StringUtil::split(getenv("PATH"), ":");
#endif

	if (!PATHs.size())
		return "";
	for (string path : PATHs) {
		string fullPath =
#ifdef _WIN32
			permutePathExt(combinePaths({path, program}));
#else
			combinePaths({path, program});
#endif
		if (exists(fullPath))
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
		else if (i.empty() && normd.size() > 0) // preserve beginning '/'
			continue;
		else if (i == ".." && normd.size() > 0 && normd[normd.size() - 1] != "..")
			normd.pop_back();
		else
			normd.push_back(i);
	}
	ret = "";
	for (vector<string>::size_type i = 0; i < normd.size(); i++) {
		if (i != 0)
			ret += "/";
		ret += normd[i];
	}

#ifdef _WIN32
	// Check for Cygwin/MSYS-style path
	if (ret[0] == '/' && ret[2] == '/') {
		ret[0] = toupper(ret[1]);
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

string FSUtil::parentDirectory(const string& path)
{
	string ret = normalizePath(path);
	return ret.substr(0, ret.rfind("/", ret.length() - 1 /* in case it ends with a '/' */));
}

void FSUtil::mkdir(const string& path)
{
	// TODO: error handling?
#ifdef _MSC_VER
	_mkdir(path.c_str());
#else
	mkdir(path.c_str());
#endif
}
