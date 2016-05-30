/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Generators.h"

#include "generators/NinjaGenerator.h"

using std::string;

string Generators::defaultName()
{
	return "Ninja";
}

std::vector<std::string> Generators::list()
{
	return {"Ninja"};
}

Generator* Generators::create(string name)
{
	if (name == "Ninja")
		return new NinjaGenerator;
	return nullptr;
}
