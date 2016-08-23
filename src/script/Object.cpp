/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Object.h"

#include "Function.h"

#include "util/PrintUtil.h"

using std::string;

namespace Script {

Exception::~Exception() noexcept
{
}

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
	case InternalError:
		// 'fWhat' is the error string
		error += fWhat;
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
Object::Object(const Object& other)
	:
	function(nullptr),
	list(nullptr),
	map(nullptr),
	extradata(nullptr),
	fObjectType(other.fObjectType)
{
	boolean = other.boolean;
	integer = other.integer;
	string = other.string;
	if (other.function) function = new Function(*other.function);
	else if (other.list) list = new ObjectList(*other.list);
	else if (other.map) map = new ObjectMap(*other.map);
	extradata = other.extradata; // Yep, this never gets deleted.
}
Object& Object::operator=(const Object& other)
{
	delete function;
	delete list;
	delete map;

	fObjectType = other.fObjectType;
	boolean = other.boolean;
	integer = other.integer;
	string = other.string;
	if (other.function) function = new Function(*other.function);
	else if (other.list) list = new ObjectList(*other.list);
	else if (other.map) map = new ObjectMap(*other.map);
	extradata = other.extradata; // Yep, this never gets deleted.

	return *this;
}

Object::~Object()
{
	delete function;
	delete list;
	delete map;
}

string Object::typeName(Type type)
{
	switch (type) {
	case Type::Undefined:	return "Undefined";
	case Type::Boolean:		return "Boolean";
	case Type::Integer:		return "Integer";
	case Type::String:		return "String";
	case Type::Function:	return "Function";
	case Type::List:		return "List";
	case Type::Map:			return "Map";
	}
	return "UNKNOWN";
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
	case Type::List: {
		std::string ret("<List:[");
		for (ObjectList::size_type i = 0; i < list->size(); i++)
			ret.append((*list)[i].asStringPretty()).append(", ");
		ret.erase(ret.length() - 2);
		return ret.append("]>");
	}
	case Type::Map: {
		std::string ret("<Map:{");
		for (ObjectMap::const_iterator it = map->begin(); it != map->end(); it++)
			ret.append(it->first).append(": ").append(it->second->asStringPretty()).append(", ");
		ret.erase(ret.length() - 2);
		return ret.append("}>");
	}
	}
	return "UNKNOWN";
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
	case Type::List: {
		std::string ret("[");
		for (ObjectList::size_type i = 0; i < list->size(); i++)
			ret.append((*list)[i].asStringRaw()).append(", ");
		ret.erase(ret.length() - 2);
		return ret.append("]");
	}
	case Type::Map: {
		std::string ret("{");
		for (ObjectMap::const_iterator it = map->begin(); it != map->end(); it++)
			ret.append(it->first).append(": ").append(it->second->asStringRaw()).append(", ");
		ret.erase(ret.length() - 2);
		return ret.append("}");
	}
	}
	return "UNKNOWN";
}

bool Object::coerceToBoolean() const
{
	switch (fObjectType) {
	case Type::Undefined:	return false;
	case Type::Boolean:		return boolean;
	case Type::Integer:		return integer != 0;
	case Type::String:		return string.length() != 0;
	case Type::Function:	return true;
	case Type::List:		return list->size() != 0;
	case Type::Map:			return map->size() != 0;
	}
	return false;
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
	else if (left.type() == Type::Integer && right.type() == Type::String)
		return StringObject(std::to_string(left.integer) + right.string);
	else if (left.type() == Type::String && right.type() == Type::Integer)
		return StringObject(left.string + std::to_string(right.integer));
	else if (left.type() == Type::List) {
		Object o = ListObject(*left.list);
		o.list->push_back(right);
		return o;
	}
	throw Exception(Exception::TypeError, "unexpected operand types for add operation (left type '"
		+ left.typeName() + "', right type '" + right.typeName() + "')");
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
	else if ((left.type() == Type::Undefined) ^ (right.type() == Type::Undefined))
		return BooleanObject(false);
	else // Compare as strings
		return BooleanObject(left.asStringRaw() == right.asStringRaw());
}

Object Object::op_and(const Object& left, const Object& right)
{
	if (left.type() == Type::Undefined || right.type() == Type::Undefined)
		return BooleanObject(false);
	return BooleanObject(left.coerceToBoolean() && right.coerceToBoolean());
}
Object Object::op_or(const Object& left, const Object& right)
{
	return BooleanObject(left.coerceToBoolean() || right.coerceToBoolean());
}
Object Object::op_lt(const Object& left, const Object& right)
{
	if (left.type() == Type::Integer && right.type() == Type::Integer)
		return BooleanObject(left.integer < right.integer);
	throw Exception(Exception::TypeError, "unexpected operand types for less-than operation (left type '"
		+ left.typeName() + "', right type '" + right.typeName() + "')");
}
Object Object::op_gt(const Object& left, const Object& right)
{
	if (left.type() == Type::Integer && right.type() == Type::Integer)
		return BooleanObject(left.integer > right.integer);
	throw Exception(Exception::TypeError, "unexpected operand types for greater-than operation (left type '"
		+ left.typeName() + "', right type '" + right.typeName() + "')");
}
Object Object::op_lteq(const Object& left, const Object& right)
{
	if (left.type() == Type::Integer && right.type() == Type::Integer)
		return BooleanObject(left.integer <= right.integer);
	throw Exception(Exception::TypeError, "unexpected operand types for less-than-or-equals operation"
		"(left type '" + left.typeName() + "', right type '" + right.typeName() + "')");
}
Object Object::op_gteq(const Object& left, const Object& right)
{
	if (left.type() == Type::Integer && right.type() == Type::Integer)
		return BooleanObject(left.integer >= right.integer);
	throw Exception(Exception::TypeError, "unexpected operand types for greater-than-or-equals operation"
		"(left type '" + left.typeName() + "', right type '" + right.typeName() + "')");
}

}
