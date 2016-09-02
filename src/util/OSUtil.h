/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

class OSUtil
{
public:
	static std::string name(bool userfriendly = true);
	static bool isFamilyUnix();

	struct ExecResult {
		std::string output;
		int exitcode;
	};
	static ExecResult exec(const std::string& program, const std::string& args,
		bool forwardOutput = false);

	static std::string getEnv(const std::string& env);
};
