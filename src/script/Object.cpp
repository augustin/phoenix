/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Object.h"

#include "Function.h"

#include "util/PrintUtil.h"
#include "util/StringUtil.h"

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


CObject::CObject(const Type type)
	:
	boolean(false),
	integer(0),
	function(nullptr),
	list(nullptr),
	map(nullptr),

	fType(type)
{
}
CObject::CObject(const CObject& other)
	:
	function(nullptr),
	list(nullptr),
	map(nullptr),
	fType(other.fType)
{
	boolean = other.boolean;
	integer = other.integer;
	string = other.string;
	if (other.function) function = new Function(*other.function);
	else if (other.list) list = new ObjectList(*other.list);
	else if (other.map) map = new ObjectMap(*other.map);
}
CObject& CObject::operator=(const CObject& other)
{
	delete function;
	delete list;
	delete map;

	fType = other.fType;
	boolean = other.boolean;
	integer = other.integer;
	string = other.string;
	if (other.function) function = new Function(*other.function);
	else if (other.list) list = new ObjectList(*other.list);
	else if (other.map) map = new ObjectMap(*other.map);

	return *this;
}

CObject::~CObject()
{
	delete function;
	delete list;
	delete map;
}

string CObject::typeName(Type type)
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
string CObject::typeName() const
{
	return typeName(fType);
}

string CObject::asStringPretty() const
{
	// Modify asStringRaw() below simultaneously with this one!
	switch (fType) {
	case Type::Undefined:
		return "<Undefined>";
	case Type::Boolean:
		return std::string("<Boolean:").append(boolean ? "true>" : "false>");
	case Type::Integer:
		return std::string("<Integer:").append(std::to_string(integer)).append(">");
	case Type::String: {
		std::string fixedStr = string;
		StringUtil::replaceAll(fixedStr, "\\n", "\\\\n");
		StringUtil::replaceAll(fixedStr, "\n", "\\n");
		return std::string("<String:\"").append(fixedStr).append("\">");
	}
	case Type::Function:
		return std::string("<Function>");
	case Type::List: {
		std::string ret("<List:[");
		for (ObjectList::size_type i = 0; i < list->size(); i++)
			ret.append(list->get(i)->asStringPretty()).append(", ");
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
string CObject::asStringRaw() const
{
	// Modify asStringPretty() above simultaneously with this one!
	switch (fType) {
	case Type::Undefined:
		return "<Undefined>";
	case Type::Boolean:
		return boolean ? "true" : "false";
	case Type::Integer:
		return std::to_string(integer);
	case Type::String:
		return string;
	case Type::Function:
		return std::string("<")
			.append((function && function->isNative()) ? "Native" : "")
			.append("Function>");
	case Type::List: {
		std::string ret("[");
		for (ObjectList::size_type i = 0; i < list->size(); i++)
			ret.append(list->get(i)->asStringRaw()).append(", ");
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

bool CObject::coerceToBoolean() const
{
	switch (fType) {
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

Object CObject::op_div(const Object left, const Object right)
{
	CoerceOrThrow("left-hand side of '/'", left, Type::Integer);
	CoerceOrThrow("right-hand side of '/'", right, Type::Integer);
	return IntegerObject(left->integer / right->integer);
}
Object CObject::op_mult(const Object left, const Object right)
{
	CoerceOrThrow("left-hand side of '*'", left, Type::Integer);
	CoerceOrThrow("right-hand side of '*'", right, Type::Integer);
	return IntegerObject(left->integer * right->integer);
}
Object CObject::op_modulo(const Object left, const Object right)
{
	CoerceOrThrow("left-hand side of '%'", left, Type::Integer);
	CoerceOrThrow("right-hand side of '%'", right, Type::Integer);
	return IntegerObject(left->integer % right->integer);
}
Object CObject::op_subt(const Object left, const Object right)
{
	CoerceOrThrow("left-hand side of '-'", left, Type::Integer);
	CoerceOrThrow("right-hand side of '-'", right, Type::Integer);
	return IntegerObject(left->integer - right->integer);
}
Object CObject::op_add(const Object left, const Object right)
{
	if (left->type() == Type::Undefined && right->type() == Type::Undefined)
		throw Exception(Exception::TypeError, "undefined cannot be added to undefined");
	else if (left->type() == Type::String && right->type() == Type::String)
		return StringObject(left->string + right->string);
	else if (left->type() == Type::Integer && right->type() == Type::Integer)
		return IntegerObject(left->integer + right->integer);
	else if (left->type() == Type::Integer && right->type() == Type::String)
		return StringObject(std::to_string(left->integer) + right->string);
	else if (left->type() == Type::String && right->type() == Type::Integer)
		return StringObject(left->string + std::to_string(right->integer));
	else if (left->type() == Type::List) {
		Object o = ListObject(new ObjectList(*left->list));
		o->list->push_back(right);
		return o;
	}
	throw Exception(Exception::TypeError, "unexpected operand types for add operation (left type '"
		+ left->typeName() + "', right type '" + right->typeName() + "')");
}
Object CObject::op_eq(const Object left, const Object right)
{
	if (left->type() == Type::String && right->type() == Type::String)
		return BooleanObject(left->string == right->string);
	else if (left->type() == Type::Integer && right->type() == Type::Integer)
		return BooleanObject(left->integer == right->integer);
	else if (left->type() == Type::Boolean && right->type() == Type::Boolean)
		return BooleanObject(left->boolean == right->boolean);

	else if (left->type() == Type::Undefined && right->type() == Type::Undefined)
		return BooleanObject(true);
	else if ((left->type() == Type::Undefined) ^ (right->type() == Type::Undefined))
		return BooleanObject(false);
	else // Compare as strings
		return BooleanObject(left->asStringRaw() == right->asStringRaw());
}

Object CObject::op_and(const Object left, const Object right)
{
	if (left->type() == Type::Undefined || right->type() == Type::Undefined)
		return BooleanObject(false);
	return BooleanObject(left->coerceToBoolean() && right->coerceToBoolean());
}
Object CObject::op_or(const Object left, const Object right)
{
	return BooleanObject(left->coerceToBoolean() || right->coerceToBoolean());
}
Object CObject::op_lt(const Object left, const Object right)
{
	if (left->type() == Type::Integer && right->type() == Type::Integer)
		return BooleanObject(left->integer < right->integer);
	throw Exception(Exception::TypeError, "unexpected operand types for less-than operation (left type '"
		+ left->typeName() + "', right type '" + right->typeName() + "')");
}
Object CObject::op_gt(const Object left, const Object right)
{
	if (left->type() == Type::Integer && right->type() == Type::Integer)
		return BooleanObject(left->integer > right->integer);
	throw Exception(Exception::TypeError, "unexpected operand types for greater-than operation (left type '"
		+ left->typeName() + "', right type '" + right->typeName() + "')");
}
Object CObject::op_lteq(const Object left, const Object right)
{
	if (left->type() == Type::Integer && right->type() == Type::Integer)
		return BooleanObject(left->integer <= right->integer);
	throw Exception(Exception::TypeError, "unexpected operand types for less-than-or-equals operation"
		"(left type '" + left->typeName() + "', right type '" + right->typeName() + "')");
}
Object CObject::op_gteq(const Object left, const Object right)
{
	if (left->type() == Type::Integer && right->type() == Type::Integer)
		return BooleanObject(left->integer >= right->integer);
	throw Exception(Exception::TypeError, "unexpected operand types for greater-than-or-equals operation"
		"(left type '" + left->typeName() + "', right type '" + right->typeName() + "')");
}

ObjectList::ObjectList(const ObjectList& other)
	:
	_inherited()
{
	for (const_iterator it = other.begin(); it != other.end(); it++)
		push_back(CopyObject(*it));
}
ObjectList& ObjectList::operator=(const ObjectList& other)
{
	clear();
	for (const_iterator it = other.begin(); it != other.end(); it++)
		push_back(CopyObject(*it));
	return *this;
}

void ObjectList::set_ptr(_inherited::size_type i, const Object obj)
{
	if (i >= _inherited::size()) {
		_inherited::size_type j = _inherited::size() - 1;
		while (j < i) {
			_inherited::push_back(UndefinedObject());
			j++;
		}
	}
	_inherited::operator[](i) = obj;
}

}
