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

using std::string;
using std::vector;

bool FSUtil::exists(const string& file)
{
	string filename = file;
#if defined(_WIN32) || defined(WIN32)
	StringUtil::replaceAll(filename, "/", "\\");
#endif

	std::ifstream f(filename);
	return static_cast<bool>(f);
}

string FSUtil::which(const string& program)
{
	if (program[0] == '/'
#if defined(_WIN32) || defined(WIN32)
		|| (program[0] == '\\' || (program[1] == ':' && program[2] == '\\'))
#endif
		)
		return program; // already an absolute path
	if (program.empty())
		return program;

#if defined(_WIN32) || defined(WIN32)
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
#if defined(_WIN32) || defined(WIN32)
		|| program.find('\\') != string::npos
#endif
		) {
		string normd = normalizePath(program);
		if (exists(normd)) {
			// TODO: check if 'program' is executable
			// TODO: return absolute path
			return normd;
		}
#if defined(_WIN32) || defined(WIN32)
		else
			return permutePathExt(normalizePath(program));
#endif
		return "";
	}

	vector<string> PATHs =
#if defined(_WIN32) || defined(WIN32)
		StringUtil::split(getenv("PATH"), ";");
#else
		StringUtil::split(getenv("PATH"), ":");
#endif

	if (!PATHs.size())
		return "";
	for (string path : PATHs) {
		string fullPath =
#if defined(_WIN32) || defined(WIN32)
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
#if defined(_WIN32) || defined(WIN32)
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

#if defined(_WIN32) || defined(WIN32)
	if (ret[0] == '/') {
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
#if defined(_WIN32) || defined(WIN32)
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
