/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

#include "Object.h"
#include "Stack.h"

namespace Script {

class ExprNode
{
public:
	enum Type {
		Literal = 0,
		RawString,
		Variable,
		Operator,
	};

	inline ExprNode(Type t)
		: type(t) {}
	inline ExprNode(Type t, const Object& obj)
		: type(t), literal(obj) {}
	inline ExprNode(Type t, const std::string& str)
		: type(t), string(str) {}
	inline ExprNode(Type t, const std::vector<std::string>& var)
		: type(t), variable(var) {}

	Object toObject(Stack* stack);

	Type type;
	Object literal;
	std::string string;
	std::vector<std::string> variable;
};

ExprNode EvalVariableName(Stack* stack, const std::string& code, uint32_t& line, std::string::size_type& i);
Object EvalString(Stack* stack, const std::string& code, std::string fromPath, const uint32_t fromLine = 1,
	bool popDirs = false);
Object Run(Stack* stack, std::string path);

}
