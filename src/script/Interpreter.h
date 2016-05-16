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

// AST
class ASTNode
{
public:
	enum Type {
		Literal = 0,
		RawString,
		Variable,
		Operator,
	};

	inline ASTNode(Type t)
		: type(t) {}
	inline ASTNode(Type t, const Object& obj)
		: type(t), literal(obj) {}
	inline ASTNode(Type t, const std::string& str)
		: type(t), string(str) {}
	inline ASTNode(Type t, const std::vector<std::string>& var)
		: type(t), variable(var) {}

	Object toObject(Stack* stack);

	Type type;
	Object literal;
	std::string string;
	std::vector<std::string> variable;
};

ASTNode EvalVariableName(Stack* stack, const std::string& code, uint32_t& line, std::string::size_type& i);
Object Run(Stack* stack, std::string path);

}