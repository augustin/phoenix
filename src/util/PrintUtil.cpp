/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "PrintUtil.h"

#include <iostream>
#include <io.h>

using std::string;

bool PrintUtil::sChecking;

void PrintUtil::error(const string& str)
{
	// TODO: queue message if (sChecking)
	std::cout << "error: " << str << "\n";
}

void PrintUtil::message(const string& str)
{
	std::cout << "message: " << str << "\n";
}

void PrintUtil::checking(const std::string& str)
{
	sChecking = true;
	std::cout << "checking " << str << "... ";
}
void PrintUtil::checkFinished(const std::string& str)
{
	sChecking = false;
	std::cout << str << "\n";
}
