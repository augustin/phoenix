/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>

class PrintUtil
{
public:
	static void error(const std::string& str);
	static void message(const std::string& str);

	static void checking(const std::string& str);
	static void checkFinished(const std::string& str);

private:
	static bool sChecking;
};
