/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "PrintUtil.h"

#include <iostream>

#ifndef _WIN32
#  include <unistd.h>
#  define COLOR_RED "\e[97m\e[31m"
#  define COLOR_GREEN "\e[97m\e[32m"
#  define COLOR_YELLOW "\e[97m\e[33m"
#  define COLOR_RESET "\e[0m"
#else
#  define COLOR_RED ""
#  define COLOR_GREEN ""
#  define COLOR_YELLOW ""
#  define COLOR_RESET ""
#endif

using std::string;

bool PrintUtil::sChecking;
#ifndef _WIN32
const bool PrintUtil::sIsTTY = isatty(1) && isatty(2);
#else
const bool PrintUtil::sIsTTY = false;
#endif

void PrintUtil::error(const string& str)
{
	std::cerr << (sIsTTY ? COLOR_RED : "") <<
		"error: " << str <<
		(sIsTTY ? COLOR_RESET : "") << std::endl;
}

void PrintUtil::warning(const string& str)
{
	std::cerr << (sIsTTY ? COLOR_YELLOW : "") <<
		"warning: " << str <<
		(sIsTTY ? COLOR_RESET : "") << std::endl;
}

void PrintUtil::message(const string& str)
{
	std::cout << "message: " << str << std::endl;
}

void PrintUtil::checking(const string& str)
{
	sChecking = true;
	std::cout << "checking " << str << "... " << std::flush;
}
void PrintUtil::checkFinished(const string& str, int status)
{
	sChecking = false;
	string color = (status == 0) ? COLOR_RED : (status == 1 ? COLOR_YELLOW : COLOR_GREEN);
	std::cout << (sIsTTY ? color : "") << str <<
		(sIsTTY ? COLOR_RESET : "") << std::endl << std::flush;
}
