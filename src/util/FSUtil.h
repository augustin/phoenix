/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

class FSUtil
{
public:
	static bool exists(const std::string& path);
	static bool isFile(const std::string& path);
	static bool isDir(const std::string& path);

	static std::string getContents(const std::string& file);
	static void putContents(const std::string& file, const std::string& contents);
	static void deleteFile(const std::string& file);

	static std::vector<std::string> searchForFiles(const std::string& dir,
		const std::vector<std::string>& exts, bool recursive);

	static std::string which(const std::string& program);

	static std::string normalizePath(const std::string& path);
	static std::string combinePaths(const std::vector<std::string>& paths);
	static std::string parentDirectory(const std::string& path);

	static void mkdir(const std::string& dirname);
};
