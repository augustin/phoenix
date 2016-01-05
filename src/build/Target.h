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
	std::vector<std::string> languages;
	std::string standardsModeFlag;
};

void addGlobalFunction();
}
