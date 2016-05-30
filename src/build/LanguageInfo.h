/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <cinttypes>
#include <map>
#include <string>
#include <vector>

#include "build/Generators.h"
#include "script/Object.h"
#include "util/OSUtil.h"

// TODO: per-compiler rules, maybe?
#ifdef _WIN32
#  define OBJECT_FILE_EXT ".obj"
#  define BINARY_FILE_EXT ".exe"
#else
#  define OBJECT_FILE_EXT ".o"
#  define BINARY_FILE_EXT ""
#endif

class LanguageInfo
{
public:
	LanguageInfo(std::string langName, Script::Object info);

	static LanguageInfo* getLanguageInfo(std::string langName);

	// Basic info
	std::string name;
	std::vector<std::string> sourceExtensions;
	std::vector<std::string> extraExtensions;
	std::string compilerEnviron;

	// Compiler info
	std::string compilerName;
	std::string compilerBinary;
	std::string compilerDefaultFlags;
	std::string compilerCompileFlag;
	std::string compilerOutputFlag;
	std::string compilerLinkBinaryFlag;
	std::string compilerDefinition;
	std::string compilerInclude;

	// Standards modes
	struct StandardsMode {
		std::string test;
		std::string normalFlag;
		std::string strictFlag;
		int8_t status; // 0 is untested, -1 is doesn't work, 1 is OK
	};
	std::map<std::string, StandardsMode> standardsModes;

	// Checks
	bool checkStandardsMode(std::string standardsMode);

	// Generation
	void generate(Generator* gen);

	// From command-line flags & the like.
	static std::map<std::string, std::string> sPreferredCompiler;
private:
	bool fGenerated;

	OSUtil::ExecResult checkIfCompiles(const std::string& testName,
		const std::string& testContents, const std::string& extraFlags = "");

	static std::map<std::string, LanguageInfo*> sData;
};
