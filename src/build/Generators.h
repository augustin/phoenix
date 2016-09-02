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

// Predefinitions
class Target;

class Generator
{
public:
	virtual ~Generator();

	virtual bool check() = 0;

	// Generates the rule to regenerate the files when someone modifies a script
	virtual void setBuildScriptFiles(const std::string& program,
		const std::vector<std::string> files) = 0;

	// Rules
	/*! `rule` must be a command string, and should include the following keywords:
	 * 	 - `%INPUTFILE%`: The input file name.
	 * 	 - `%OUTPUTFILE%`: The output file name.
	 *   - `%TARGETFLAGS%`: The place to insert per-target flags.
	 */
	virtual void addRegularRule(const std::string& ruleName,
		const std::string& descName, const std::vector<std::string>& forExts,
		const std::string& program, const std::string& outFileExt,
	    const std::string& rule) = 0;
	virtual void addLinkRule(const std::string& ruleName,
		const std::string& descName, const std::string& program,
		const std::string& rule) = 0;

	virtual void addTarget(const std::string& linkRule,
		const std::string& outputBinaryName,
		const std::vector<std::string>& inputFiles,
	    const std::string& targetFlags, Target* target) = 0;

	virtual std::vector<std::string> outputFiles() = 0;
	virtual std::string command(const std::string& target = "");

	virtual void write() = 0;
};

class Generators
{
public:
	// Don't touch unless you know what you're doing.
	static Generator* primary;
	static Generator* actual;

	static std::string defaultName();
	static std::vector<std::string> list();
	static std::vector<std::string> listSecondary();
	static Generator* create(std::string name, std::vector<std::string> secondary = {});
};
