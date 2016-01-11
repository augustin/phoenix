/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Tester.h"

#include <iostream>

#ifdef _WIN32
#  define COLOR_RED ""
#  define COLOR_GREEN ""
#  define COLOR_RESET ""
#else
#  define COLOR_RED "\e[97m\e[101m"
#  define COLOR_GREEN "\e[97m\e[42m"
#  define COLOR_RESET "\e[0m"
#endif

Tester::Tester(bool hidePassed)
	: fTestsPassed(0),
	  fTestsFailed(0),
	  fHidePassed(hidePassed)
{
}

void Tester::beginGroup(const std::string& groupName)
{
	if (!fHidePassed)
		std::cout << groupName << std::endl;
	fCurGroup = groupName;
	fGroupPrinted = false;
}
void Tester::endGroup()
{
	if (!fHidePassed || fGroupPrinted)
		std::cout << std::endl;
	fCurGroup = "";
	fGroupPrinted = false;
}

void Tester::result(bool pass, const std::string& testName)
{
	if (pass && fHidePassed) {
		fTestsPassed++;
		return;
	}

	if (fHidePassed && !fGroupPrinted) {
		std::cout << fCurGroup << std::endl;
		fGroupPrinted = true;
	}

	if (pass) {
		std::cout << COLOR_GREEN << "PASS" << COLOR_RESET;
		fTestsPassed++;
	} else {
		std::cout << COLOR_RED << "FAIL" << COLOR_RESET;
		fTestsFailed++;
	}
	std::cout << ": " << testName << std::endl;
}

int Tester::done()
{
	std::cout << "Finished: " << std::to_string(fTestsPassed) <<
		" passed, " << std::to_string(fTestsFailed) << " failed." << std::endl <<
		std::endl;
	return fTestsFailed;
}
