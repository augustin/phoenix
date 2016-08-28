/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>

class PrintUtil
{
public:
	static void error(const std::string& str);
	static void warning(const std::string& str);
	static void message(const std::string& str);

	static void checking(const std::string& str);
	// 0 is failed, 1 is warning, 2 is succeeded
	static void checkFinished(const std::string& str, int status = 2);

private:
	static bool sChecking;
	static const bool sIsTTY;
};
