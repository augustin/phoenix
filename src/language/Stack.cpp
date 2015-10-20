/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Stack.h"

using std::string;

namespace Language {

std::map<std::string, Function> GlobalFunctions;

Stack::Stack()
{
}

Object Stack::get(string variableName)
{
	if (variableName[0] == '$') {
		// it's a superglobal
		variableName.erase(1);
		if (fSuperglobalScope.get(variableName).type() != Type::Nonexistent)
			return fSuperglobalScope.get(variableName);
		else
			return Object(); // undefined
	}
	if (fCurrentScope.get(variableName).type() != Type::Nonexistent)
		return fCurrentScope.get(variableName);
	if (fStack.size() > 0) for (ObjectMap::size_type i = fStack.size(); i >= 0; i--) {
		Object ret = fStack[i].get(variableName);
		if (ret.type() != Type::Nonexistent)
			return ret;
	}

	return Object(); // undefined
}

void Stack::set(string variableName, Object value)
{
	if (variableName[0] == '$') {
		// it's a superglobal
		throw Exception(Exception::AccessViolation, "superglobals are read-only");
	}
	if (fCurrentScope.get(variableName).type() != Type::Nonexistent) {
		fCurrentScope.set(variableName, value);
		return;
	}
	if (fStack.size() > 0) for (ObjectMap::size_type i = fStack.size() - 1; i >= 0; i--) {
		if (fStack[i].get(variableName).type() != Type::Nonexistent) {
			fStack[i].set(variableName, value);
			return;
		}
	}
	fCurrentScope.set(variableName, value);
}

void Stack::addSuperglobal(string variableName, Object value)
{
	fSuperglobalScope.set(variableName, value);
}

}
