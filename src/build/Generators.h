/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

// GCC 4.6 doesn't support "override", so compensate for that
#if __GNUC__ == 4 && __GNUC_MINOR__ < 7
#  define override
#endif

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
	virtual void addObjectRule(const std::string& ruleName,
		const std::string& descName, const std::vector<std::string>& forExts,
		const std::string& program, const std::string& outFileExt,
		const std::string& rule) = 0;
	virtual void setProgramLinkRule(const std::string& rule) = 0;

	virtual void addTarget(const std::string& outputBinaryName,
		const std::vector<std::string>& inputFiles,
		const std::string& targetFlags) = 0;

	virtual void write() = 0;
};

class Generators
{
public:
	static std::string defaultName();
	static Generator* create(std::string name);
};
