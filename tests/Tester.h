/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>

class Tester
{
public:
	Tester(bool hidePassed = false);

	void beginGroup(const std::string& groupName);
	void endGroup();
	void result(bool pass, const std::string& testName);
	int done();

private:
	int fTestsPassed;
	int fTestsFailed;

	bool fHidePassed;
	std::string fCurGroup;
	bool fGroupPrinted;
};
