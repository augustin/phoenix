/*
 * (C) 2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Object.h"

using std::string;

namespace Language {

Object::Object()
	:
	fObjectType(Type::Undefined)
{
}

Object::Object(const Type type)
	:
	fObjectType(type)
{
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
	default:				return "Unknown";
	}
}

BooleanObject::BooleanObject(const bool val)
	:
	Object(Type::Boolean),
	value(val)
{
}


IntegerObject::IntegerObject(const int val)
	:
	Object(Type::Integer),
	value(val)
{
}


StringObject::StringObject(const string& str)
	:
	Object(Type::String),
	string(str)
{
}

};
