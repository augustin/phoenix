/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Stack.h"

using std::string;

namespace Language {

Stack::Stack()
{
}

Object Stack::get(string variableName)
{
	if (variableName[0] == '$') {
		// it's a superglobal
		variableName.erase(1);
		if (fSuperglobalScope.find(variableName) != fSuperglobalScope.end())
			return fSuperglobalScope[variableName];
		else
			return Object(); // undefined
	}
	if (fCurrentScope.find(variableName) != fCurrentScope.end())
		return fCurrentScope[variableName];
	for (VariableMap::size_type i = fStack.size(); i >= 0; i--) {
		if (fStack[i].find(variableName) != fStack[i].end())
			return fStack[i][variableName];
	}

	return Object(); // undefined
}

void Stack::set(string variableName, Object value)
{
	if (variableName[0] == '$') {
		// it's a superglobal
		throw Exception(Exception::AccessViolation, "Superglobals are read-only.");
	}
	if (fCurrentScope.find(variableName) != fCurrentScope.end()) {
		fCurrentScope[variableName] = value;
		return;
	}
	for (VariableMap::size_type i = fStack.size(); i >= 0; i--) {
		if (fStack[i].find(variableName) != fStack[i].end()) {
			fStack[i][variableName] = value;
			return;
		}
	}
	fCurrentScope.insert({variableName, value});
}

void Stack::addSuperglobal(string variableName, Object value)
{
	fSuperglobalScope.insert({variableName, value});
}

};
