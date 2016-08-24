/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Stack.h"

#include <iostream>

#include "util/StringUtil.h"

using std::cout;
using std::string;
using std::vector;

namespace Script {

Stack::Stack()
{
	push();
}

void Stack::push()
{
	fStack.push_back(ObjectMap());
}

void Stack::pop()
{
	fStack.pop_back();
}

std::vector<ObjectMap>::size_type Stack::getPos(std::string variable)
{
	std::vector<ObjectMap>::size_type last = fStack.size() - 1;
	for (std::vector<ObjectMap>::size_type i = 0; i <= last; i++) {
		if (fStack[last - i].get_ptr(variable) != nullptr)
			return last - i;
	}
	return last;
}

Object* Stack::get_ptr(const vector<string> variable)
{
	Object* ret = nullptr;
	if (variable[0][0] == '$') {
		// it's a superglobal
		string var = variable[0];
		var = var.substr(1);
		ret = fSuperglobalScope.get_ptr(var);
	} else
		ret = fStack[getPos(variable[0])].get_ptr(variable[0]);
	for (vector<string>::size_type i = 1; i < variable.size(); i++) {
		if (ret == nullptr)
			return new Object(Type::Undefined);
		if (ret->type() == Type::Map)
			ret = ret->map->get_ptr(variable[i]);
		else if (ret->type() == Type::List) {
			if (variable[i] == "length") {
				Object* newRet = new Object(Type::Integer);
				newRet->integer = ret->list->size();
				ret = newRet;
			} else {
				try {
					ret = ret->list->get_ptr(std::stoi(variable[i], nullptr, 10));
				} catch (...) {
					throw Exception(Exception::SyntaxError,
						string("expected integer, got '").append(variable[i]).append("'"));
				}
			}
		} else
			throw Exception(Exception::TypeError, "'" + variable[i - 1] + "' should be either 'List' or 'Map' "
				"but is neither");
	}

	return ret;
}

void Stack::set_ptr(vector<string> variable, Object* value)
{
	if (variable[0][0] == '$') {
		// it's a superglobal
		throw Exception(Exception::AccessViolation, "superglobals are read-only");
	}
	vector<ObjectMap>::size_type loc = getPos(variable[0]);
	if (variable.size() == 1) {
		fStack[loc].set_ptr(variable[0], value);
		return;
	}

	Object* res = fStack[loc].get_ptr(variable[0]);
	CoerceOrThrowPtr("referenced variable", res, Type::Map);
	for (vector<string>::size_type i = 1; i < variable.size() - 1; i++) {
		res = res->map->get_ptr(variable[i]);
		CoerceOrThrowPtr("referenced variable", res, Type::Map);
	}
	res->map->set_ptr(variable[variable.size() - 1], value);
}

void Stack::addSuperglobal(string variableName, Object value)
{
	fSuperglobalScope.set(variableName, value);
}

void Stack::print()
{
	cout << "-- VM STACK DUMP --" << std::endl;
	int tabs = 1;
	for (const ObjectMap& m : fStack) {
		for (ObjectMap::const_iterator it = m.begin(); it != m.end(); it++) {
			for (int i = 0; i < tabs; i++)
				cout << "    ";
			cout << it->first << ": " << it->second->asStringPretty() << std::endl;
		}
		tabs++;
	}
}

}
