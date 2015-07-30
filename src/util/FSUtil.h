/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

class FSUtil
{
public:
	static bool exists(const std::string& file);
	static std::string which(const std::string& program);

	static std::string normalizePath(const std::string& path);
	static std::string combinePaths(const std::vector<std::string>& paths);
};
