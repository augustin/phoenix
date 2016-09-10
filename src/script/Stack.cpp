/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Stack.h"

#include <iostream>

#include "script/Builtins.h"
#include "util/StringUtil.h"

using std::cout;
using std::string;
using std::vector;

namespace Script {

Stack::Stack()
{
	push();
	addSuperglobal("Phoenix", std::make_shared<Script::GlobalPhoenixObject>(this));
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

Object Stack::get_ptr(const vector<string> variable)
{
	Object ret = nullptr;
	if (variable[0][0] == '$') {
		// it's a superglobal
		string var = variable[0];
		var = var.substr(1);
		ret = fSuperglobalScope.get_ptr(var);
	} else
		ret = fStack[getPos(variable[0])].get_ptr(variable[0]);
	for (vector<string>::size_type i = 1; i < variable.size(); i++) {
		if (ret == nullptr)
			return UndefinedObject();

		Object primMember;
		if ((ret->type() == Type::Map || ret->type() == Type::List || ret->type() == Type::String) &&
			(primMember = ret->primitiveMember(variable[i]))->type() != Type::Undefined) {
			return primMember;
		} else if (ret->type() == Type::Map)
			ret = ret->map->get_ptr(variable[i]);
		else if (ret->type() == Type::List) {
			try {
				ret = ret->list->get_ptr(std::stoi(variable[i], nullptr, 10));
			} catch (...) {
				throw Exception(Exception::SyntaxError,
					string("expected integer, got '").append(variable[i]).append("'"));
			}
		} else
			throw Exception(Exception::TypeError, "'" + variable[i - 1] + "' should be either 'List' or 'Map' "
				"but is neither");
	}

	return ret;
}

void Stack::set_ptr(vector<string> variable, Object value, bool forceLocal)
{
	if (variable[0][0] == '$') {
		// it's a superglobal
		throw Exception(Exception::AccessViolation, "superglobals are read-only");
	}
	if (forceLocal)
		fStack[fStack.size() - 1].set_ptr(variable[0], value);

	vector<ObjectMap>::size_type loc = getPos(variable[0]);
	if (variable.size() == 1) {
		fStack[loc].set_ptr(variable[0], value);
		return;
	}

	Object res = fStack[loc].get_ptr(variable[0]);
	if (res->type() != Type::Map && res->type() != Type::List)
		throw Exception(Exception::TypeError, "'" + variable[0] + "' should be either 'List' or 'Map' "
			"but is neither");
	for (vector<string>::size_type i = 1; i < variable.size() - 1; i++) {
		if (res->type() == Type::Map) {
			res = res->map->get_ptr(variable[i]);
		} else if (res->type() == Type::List) {
			try {
				res = res->list->get_ptr(std::stoi(variable[i]));
			} catch (...) {
				throw Exception(Exception::SyntaxError, "'" + variable[i - 1] + "' is of type 'List,"
					"but the program attempted to reference a non-integer");
			}
		}
		if (res->type() != Type::Map && res->type() != Type::List)
			throw Exception(Exception::TypeError, "'" + variable[i - 1] + "' should be either 'List' or 'Map' "
				"but is neither");
	}
	if (res->type() == Type::Map)
		res->map->set_ptr(variable[variable.size() - 1], value);
	else if (res->type() == Type::List) {
		try {
			res->list->set(std::stoi(variable[variable.size() - 1]), value);
		} catch (...) {
			throw Exception(Exception::SyntaxError, "'" + variable[variable.size() - 1] + "' is of type 'List,"
				"but the program attempted to reference a non-integer");
		}
	}
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
