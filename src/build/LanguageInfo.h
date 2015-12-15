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

	std::string fName;
	std::vector<std::string> fSourceExtensions;
	std::vector<std::string> fExtraExtensions;
	bool fPreprocessor;
	std::string fCompilerEnviron;

	// Compiler info
	std::string fCompilerName;
	std::string fCompilerBinary;
	std::string fCompilerCompile;
	std::string fCompilerDefinition;
	std::string fCompilerInclude;

private:
	static std::map<std::string, LanguageInfo*> sData;
};
