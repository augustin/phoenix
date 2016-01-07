/*
 * (C) 2015-2016 Augustin Cavalier
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
		AccessViolation,

		UserError,
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
};

class Object
{
public:
	Object(const Type type = Type::Undefined);
	~Object();

	inline Type type() const { return fObjectType; }
	static std::string typeName(Type type);
	std::string typeName() const;
	std::string asStringPretty() const;
	std::string asStringRaw() const;

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
	std::string string;
	Function* function;
	std::vector<Object>* list;
	ObjectMap* map;
	void* extradata;

private:
	Type fObjectType;
};

// Helper functions
inline void CoerceOrThrow(const std::string& what, const Object& variable, Type type)
{
	if (variable.type() != type) {
		throw Exception(Exception::TypeError,
			std::string(what).append(" should be of type '")
				.append(Object::typeName(type)).append("' but is of type '")
				.append(variable.typeName()).append("'"));
	}
}
inline void CoerceOrThrowPtr(const std::string& what, const Object* variable, Type type)
{
	if (variable == nullptr) {
		throw Exception(Exception::TypeError,
			std::string(what).append(" should be of type '").append(Object::typeName(type))
				.append("' but is of type 'Undefined'"));
	}
	if (variable->type() != type) {
		throw Exception(Exception::TypeError,
			std::string(what).append(" should be of type '")
				.append(Object::typeName(type)).append("' but is of type '")
				.append(variable->typeName()).append("'"));
	}
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
	ret.string = std::string(value);
	return ret;
}
inline Object ListObject(std::vector<Object>* value)
{
	Object ret(Type::List);
	ret.list = value;
	return ret;
}

}
