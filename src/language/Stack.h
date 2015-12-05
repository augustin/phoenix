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

	void push();
	void pop();

	Object get(const std::vector<std::string> variable);
	inline Object get(const std::string variable0) {
		std::vector<std::string> variable = {variable0};
		return get(variable); }
	void set(std::vector<std::string> variable, Object value);

	void addSuperglobal(std::string variableName, Object value);

	void print();

private:
	ObjectMap fSuperglobalScope;
	std::vector<ObjectMap> fStack;

	std::vector<ObjectMap>::size_type getPos(std::string variable);
};

}
