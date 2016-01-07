/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

class Generator
{
public:
	virtual std::string name() = 0;

	// Rules
	/*! `rule` must be a command string, and should include the following keywords:
	 * 	 - `%INPUTFILE%`: The input file name.
	 * 	 - `%OUTPUTFILE%`: The output file name.
	 *   - `%TARGETFLAGS%`: The place to insert per-target flags.
	 */
	virtual void addObjectRule(std::string ruleName, std::string descName,
		std::vector<std::string> forExts, std::string binary,
		std::string outFileExt, std::string rule) = 0;
	virtual void setProgramLinkRule(std::string rule) = 0;

	virtual void addTarget(std::string outputBinaryName,
		std::vector<std::string> inputFiles) = 0;

	virtual void write() = 0;
};

class Generators
{
public:
	static std::string defaultName();
	static Generator* create(std::string name);
};
