/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

class StringUtil
{
public:
	static int count(const std::string& str, const std::string& thing);
	static std::vector<std::string> split(const std::string& str,
		const std::string& delimiter);
	static std::string join(const std::vector<std::string>& array,
		const std::string& delimiter, bool skipEmptyStrings = true);

	static std::string trim(const std::string& str);

	static bool startsWith(const std::string& haystack, const std::string& needle);
	static bool endsWith(const std::string& haystack, const std::string& needle);

	static void replaceAll(std::string& subject,
		const std::string& search, const std::string& replace);
};
