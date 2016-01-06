/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Object.h"

#include "Function.h"
#include "ObjectMap.h"

#include "util/PrintUtil.h"

using std::string;

namespace Language {

void Exception::print()
{
	string error;
	switch (fType) {
	case FileDoesNotExist:
		// 'fWhat' is the file(path)
		error += "file '" + fWhat + "' does not exist";
	break;
	case UserError:
	case SyntaxError:
	case TypeError:
	case AccessViolation:
		// 'fWhat' is the error string
		error += fWhat;
	break;
	default:
		error += "unknown error " + std::to_string(fType) + ": \"" + fWhat + "\"";
	break;
	}
	if (!fFile.empty() && fLine > 0) {
		error += " at line " + std::to_string(fLine) + " of '" + fFile + "'";
	}
	error += ".";
	PrintUtil::error(error);
}


Object::Object(const Type type)
	:
	boolean(false),
	integer(0),
	function(nullptr),
	list(nullptr),
	map(nullptr),
	extradata(nullptr),

	fObjectType(type)
{
}
Object::~Object()
{
	//delete function; // FIXME: LEAK LEAK LEAK LEAK!
	//delete list;
	//delete map;
}

string Object::typeName(Type type)
{
	switch (type) {
	case Type::Undefined:	return "Undefined";
	case Type::Boolean:		return "Boolean";
	case Type::Integer:		return "Integer";
	case Type::String:		return "String";
	case Type::Function:	return "Function";
	case Type::Map:			return "Map";
	default:				return "Unknown";
	}
}
string Object::typeName() const
{
	return typeName(fObjectType);
}

string Object::asStringPretty() const
{
	// Modify asStringRaw() below simultaneously with this one!
	switch (fObjectType) {
	case Type::Undefined:
		return "<Undefined>";
	case Type::Boolean:
		return std::string("<Boolean:").append(boolean ? "true>" : "false>");
	case Type::Integer:
		return std::string("<Integer:").append(std::to_string(integer)).append(">");
	case Type::String:
		return std::string("<String:\"").append(string).append("\">");
	case Type::Function:
		return std::string("<Function>");
	case Type::Map:
		return std::string("<Map[...]>"); // TODO
	default:
		return "<UNKNOWN>";
	}
}
string Object::asStringRaw() const
{
	// Modify asStringPretty() above simultaneously with this one!
	switch (fObjectType) {
	case Type::Undefined:
		return "<Undefined>";
	case Type::Boolean:
		return boolean ? "true" : "false";
	case Type::Integer:
		return std::to_string(integer);
	case Type::String:
		return string;
	case Type::Function:
		return std::string("<Function>");
	case Type::Map:
		return std::string("<Map[...]>"); // TODO
	default:
		return "<UNKNOWN>";
	}
}

Object Object::op_div(const Object& left, const Object& right)
{
	CoerceOrThrow("left-hand side of '/'", left, Type::Integer);
	CoerceOrThrow("right-hand side of '/'", right, Type::Integer);
	return IntegerObject(left.integer / right.integer);
}
Object Object::op_mult(const Object& left, const Object& right)
{
	CoerceOrThrow("left-hand side of '*'", left, Type::Integer);
	CoerceOrThrow("right-hand side of '*'", right, Type::Integer);
	return IntegerObject(left.integer * right.integer);
}
Object Object::op_subt(const Object& left, const Object& right)
{
	CoerceOrThrow("left-hand side of '-'", left, Type::Integer);
	CoerceOrThrow("right-hand side of '-'", right, Type::Integer);
	return IntegerObject(left.integer - right.integer);
}
Object Object::op_add(const Object& left, const Object& right)
{
	if (left.type() == Type::Undefined && right.type() == Type::Undefined)
		throw Exception(Exception::TypeError, "undefined cannot be added to undefined");
	else if (left.type() == Type::String && right.type() == Type::String)
		return StringObject(left.string + right.string);
	else if (left.type() == Type::Integer && right.type() == Type::Integer)
		return IntegerObject(left.integer + right.integer);
	throw Exception(Exception::InternalError, "unimplemented: adding strings to numbers & vice versa");
}
Object Object::op_eq(const Object& left, const Object& right)
{
	if (left.type() == Type::String && right.type() == Type::String)
		return BooleanObject(left.string == right.string);
	else if (left.type() == Type::Integer && right.type() == Type::Integer)
		return BooleanObject(left.integer == right.integer);
	else if (left.type() == Type::Boolean && right.type() == Type::Boolean)
		return BooleanObject(left.boolean == right.boolean);

	else if (left.type() == Type::Undefined && right.type() == Type::Undefined)
		return BooleanObject(true);
	else if ((left.type() == Type::Undefined && right.type() != Type::Undefined) ||
			 (left.type() != Type::Undefined && right.type() == Type::Undefined))
		return BooleanObject(false);
	throw Exception(Exception::InternalError, "unimplemented: complex type comparison");
}

bool coerceToBoolean(const Object& obj)
{
	switch (obj.type()) {
	case Type::Undefined:	return false;
	case Type::Boolean:		return obj.boolean;
	case Type::Integer:		return obj.integer != 0;
	case Type::String:		return obj.string.length() != 0;
	case Type::Function:	return true;
	case Type::Map:			return obj.map->size() != 0;
	default:
		throw Exception(Exception::InternalError, "unexpected type for lowering");
	}
}
Object Object::op_and(const Object& left, const Object& right)
{
	if (left.type() == Type::Undefined || right.type() == Type::Undefined)
		return BooleanObject(false);
	return BooleanObject(coerceToBoolean(left) && coerceToBoolean(right));
}
Object Object::op_or(const Object& left, const Object& right)
{
	return BooleanObject(coerceToBoolean(left) || coerceToBoolean(right));
}

}
