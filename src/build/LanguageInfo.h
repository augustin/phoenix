/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "language/Object.h"

class LanguageInfo
{
public:
	LanguageInfo(std::string langName, Language::Object info);

	static LanguageInfo* getLanguageInfo(std::string langName);

	std::string name;
	std::vector<std::string> sourceExtensions;
	std::vector<std::string> extraExtensions;
	bool preprocessor;
	std::string compilerEnviron;

	// Compiler info
	std::string compilerName;
	std::string compilerBinary;
	std::string compilerCompile;
	std::string compilerDefinition;
	std::string compilerInclude;

private:
	static std::map<std::string, LanguageInfo*> sData;
};
