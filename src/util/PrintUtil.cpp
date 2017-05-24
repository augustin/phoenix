/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "PrintUtil.h"

#include <iostream>

#include "TermUtil.h"

using std::string;

bool PrintUtil::sChecking = false;

void PrintUtil::error(const string& str)
{
	TermUtil::setColor(TermUtil::Red);
	std::cerr << "error: ";
	TermUtil::resetColors();
	std::cerr << str << std::endl;
}

void PrintUtil::warning(const string& str)
{
	TermUtil::setColor(TermUtil::Yellow);
	std::cerr << "warning: ";
	TermUtil::resetColors();
	std::cerr << str << std::endl;
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
	if (status == 0)
		TermUtil::setColor(TermUtil::Red);
	else if (status == 1)
		TermUtil::setColor(TermUtil::Yellow);
	else
		TermUtil::setColor(TermUtil::Green);
	std::cout << str;
	TermUtil::resetColors();
	std::cout << std::endl << std::flush;
}
