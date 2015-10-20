/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Function.h"
#include "Object.h"

namespace Language {

extern std::map<std::string, Function> GlobalFunctions;

class Stack
{
public:
	Stack();

	Object get(std::string variableName);
	void set(std::string variableName, Object value);

	void addSuperglobal(std::string variableName, Object value);

private:
	ObjectMap fSuperglobalScope;
	std::vector<ObjectMap> fStack;
	ObjectMap fCurrentScope;
};

}
