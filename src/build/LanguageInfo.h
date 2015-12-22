/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <cinttypes>
#include <map>
#include <string>
#include <vector>

#include "language/Object.h"

// TODO: per-compiler rules, maybe?
#ifdef _WIN32
#define OBJECT_FILE_EXT ".obj"
#else
#define OBJECT_FILE_EXT ".o"
#endif

class LanguageInfo
{
public:
	LanguageInfo(std::string langName, Language::Object info);

	static LanguageInfo* getLanguageInfo(std::string langName);

	// Basic info
	std::string name;
	std::vector<std::string> sourceExtensions;
	std::vector<std::string> extraExtensions;
	bool preprocessor;
	std::string compilerEnviron;

	// Compiler info
	std::string compilerName;
	std::string compilerBinary;
	std::string compilerCompileFlag;
	std::string compilerOutputFlag;
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

	// From command-line flags & the like.
	static std::map<std::string, std::string> sPreferredCompiler;
private:
	static std::map<std::string, LanguageInfo*> sData;
};
