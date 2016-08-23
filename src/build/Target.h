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

namespace Target {
struct ExtraData {
	std::string name;
	std::vector<std::string> languages;
	std::string standardsModeFlag;
	std::string definitionsFlags;
	std::string includesFlags;
	std::string otherFlags;
	std::vector<std::string> sourceFiles;
};
extern std::vector<ExtraData*> targets;

void generate(ExtraData* target, Generator* gen);

void addGlobalFunction(Script::Stack* stack);
}
