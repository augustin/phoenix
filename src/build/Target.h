/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>

#include "language/Object.h"

class Target : private Language::Object
{
public:
	Target(const Language::ObjectMap& params);

	static void addGlobalFunction();
};
