/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Object.h"

namespace Language {

class Stack
{
public:
	Stack();

	Object get(std::string variableName);
	void set(std::string variableName, Object value);

	void addSuperglobal(std::string variableName, Object value);

private:
	typedef std::map<std::string, Object> VariableMap;
	VariableMap fSuperglobalScope;
	std::vector<VariableMap> fStack;
	VariableMap fCurrentScope;
};

};
