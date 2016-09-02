/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

#include "script/Object.h"

// Predefinitions
class Generator;
namespace Script { class Stack; }

class Target
{
public:
	static std::vector<Target*> targets;
	static void addGlobalFunction(Script::Stack* stack);

	std::string name;
	std::vector<std::string> languages;
	std::string standardsModeFlag;
	std::string definitionsFlags;
	std::vector<std::string> includeDirs;
	std::string otherFlags;
	std::vector<std::string> sourceFiles;
	std::vector<std::string> extraFiles;

	void generate(Generator* gen);

private:
	Target(const Script::ObjectMap& params);
	Script::ObjectMap* fMapObject;
};
