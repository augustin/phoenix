/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

#include "language/Object.h"

namespace Target {
struct ExtraData {
	std::string name;
	std::vector<std::string> languages;
	std::string standardsModeFlag;
	std::string definitionsFlags;
	std::vector<std::string> sourceFiles;
};

void addGlobalFunction();
}
