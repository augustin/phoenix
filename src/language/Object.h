/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <exception>
#include <string>
#include <vector>

namespace Language {

// Predefinitions
class Function;
class ObjectMap;

class Exception : public std::exception
{
public:
	enum Type {
		InternalError = 2, // number here is return code, 1 is generic
		FileDoesNotExist,
		SyntaxError,
		TypeError,
		AccessViolation
	};
	Exception(Type type, std::string what, std::string file = "", uint32_t line = 0)
		: fType(type), fWhat(what), fFile(file), fLine(line) {}
	virtual ~Exception() throw() {}
	virtual const char* what() const throw() { return fWhat.c_str(); }

	void print();

public:
	Type fType;
	std::string fWhat;
	std::string fFile;
	uint32_t fLine;
};


enum class Type {
	Undefined = 0,
	Boolean,
	Integer,
	String,
	Function,
	List,
	Map,

	// Used by the parser/evaluator
	Variable,
	Operator,
	Nonexistent
};

class Object
{
public:
	Object(const Type type = Type::Undefined);
	~Object();

	inline Type type() const { return fObjectType; }
	std::string typeName() const;
	std::string asString() const;

	// Operators
	static Object op_div(const Object& left, const Object& right);
	static Object op_mult(const Object& left, const Object& right);
	static Object op_subt(const Object& left, const Object& right);
	static Object op_add(const Object& left, const Object& right);
	static Object op_eq(const Object& left, const Object& right);
	inline static Object op_neq(const Object& left, const Object& right)
		{ Object o = op_eq(left, right); o.boolean = !o.boolean; return o; }
	static Object op_and(const Object& left, const Object& right);
	static Object op_or(const Object& left, const Object& right);

public: // Data storage
	bool boolean;
	int integer;
	std::string* string; bool string_dereferenced;
	Function* function;
	ObjectMap* map;

private:
	Type fObjectType;
};

// Helper macros
#define Language_POSSIBLY_DEREFERENCE(OBJECT) \
	(OBJECT.type() == ::Language::Type::Variable) ? \
		stack->get(*OBJECT.string) : OBJECT
#define Language_COERCE_OR_THROW(WHAT, VARIABLE, TYPE) \
	if (VARIABLE.type() != ::Language::Type::TYPE) { \
		throw Exception(Exception::TypeError, \
			std::string(WHAT " should be of type '" #TYPE "' but is of type '") \
				.append(VARIABLE.typeName()).append("'")); \
	}

// Convenience constructors
inline Object BooleanObject(const bool value)
{
	Object ret(Type::Boolean);
	ret.boolean = value;
	return ret;
}
inline Object IntegerObject(const int value)
{
	Object ret(Type::Integer);
	ret.integer = value;
	return ret;
}
inline Object StringObject(const std::string& value)
{
	Object ret(Type::String);
	ret.string = new std::string(value);
	return ret;
}
// Internal parser types
inline Object VariableObject(const std::string& variableName)
{
	Object ret(Type::Variable);
	ret.string = new std::string(variableName);
	return ret;
}
inline Object OperatorObject(const std::string& oper)
{
	Object ret(Type::Operator);
	ret.string = new std::string(oper);
	return ret;
}

}
