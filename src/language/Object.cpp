/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Object.h"

#include "Function.h"
#include "ObjectMap.h"

using std::string;

namespace Language {

void Exception::print()
{
	std::cout << "error: ";
	switch (fType) {
	case FileDoesNotExist:
		// 'fWhat' is the file(path)
		std::cout << "file '" << fWhat << "' does not exist";
	break;
	case SyntaxError:
	case TypeError:
	case AccessViolation:
		// 'fWhat' is the error string
		std::cout << fWhat;
	break;
	default:
		std::cout << "unknown error " << fType << ": \"" << fWhat << "\"";
	break;
	}
	if (!fFile.empty() && fLine > 0) {
		std::cout << " at line " << fLine << " of '" << fFile << "'";
	}
	std::cout << ".\n";
}


Object::Object(const Type type)
	:
	boolean(false),
	integer(0),
	string(nullptr), string_dereferenced(true),
	function(nullptr),
	map(nullptr),

	fObjectType(type)
{
}
Object::~Object()
{
	delete string;
	delete function;
	delete map;
}

string Object::typeName()
{
	switch (fObjectType) {
	case Type::Undefined:	return "Undefined";
	case Type::Boolean:		return "Boolean";
	case Type::Integer:		return "Integer";
	case Type::String:		return "String";
	case Type::Function:	return "Function";
	case Type::Map:			return "Map";

	case Type::Variable:	return "<IPT:Variable>";
	case Type::Operator:	return "<IPT:Operator>";
	default:				return "Unknown";
	}
}

};
