/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <exception>
#include <map>
#include <string>
#include <vector>

namespace Script {

// Predefinitions
class Function;
class ObjectMap;
class ObjectList;

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
	virtual ~Exception() noexcept;
	virtual const char* what() const noexcept { return fWhat.c_str(); }

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
	// Copy constructors
	Object(const Object& other);
	Object& operator=(const Object& other);

	inline Type type() const { return fObjectType; }
	static std::string typeName(Type type);
	std::string typeName() const;
	std::string asStringPretty() const;
	std::string asStringRaw() const;

	inline Object operator[](const char* key) const;

	// Helpers
	bool coerceToBoolean() const;
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
	static Object op_lt(const Object& left, const Object& right);
	static Object op_gt(const Object& left, const Object& right);
	static Object op_lteq(const Object& left, const Object& right);
	static Object op_gteq(const Object& left, const Object& right);

public: // Data storage
	bool boolean;
	int32_t integer;
	std::string string;
	Function* function;
	ObjectList* list;
	ObjectMap* map;

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
				.append("' but is of type 'undefined'"));
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

// ObjectMap (defined here, implemented in ObjectMap.cpp)
class ObjectMap : private std::map<std::string, Object*>
{
	typedef std::map<std::string, Object*> _inherited;
public:
	ObjectMap();
	~ObjectMap();

	typedef _inherited::const_iterator const_iterator;
	const_iterator begin() const { return _inherited::begin(); }
	const_iterator end() const { return _inherited::end(); }

	Object get(std::string key) const;
	Object* get_ptr(std::string key);
	void set_ptr(std::string key, Object* value);
	inline void set(std::string key, Object value) { set_ptr(key, new Object(value)); }

	size_type size() const { return _inherited::size(); }
};

class ObjectList : private std::vector<Object*>
{
	typedef std::vector<Object*> _inherited;
public:
	ObjectList() {}
	~ObjectList() { /* for (Object* obj : *this) delete obj; // FIXME */ }

	typedef _inherited::const_iterator const_iterator;
	typedef _inherited::size_type size_type;
	const_iterator begin() const { return _inherited::begin(); }
	const_iterator end() const { return _inherited::end(); }

	void push_back(const Object& obj) { Object* o = new Object(obj); _inherited::push_back(o); }

	Object operator[](_inherited::size_type i) { return *get_ptr(i); }
	Object* get_ptr(_inherited::size_type i) { return _inherited::at(i); }

	size_type size() const { return _inherited::size(); }
};

// Must be down here, as it needs ListObject's definition
inline Object ListObject(ObjectList* value)
{
	Object ret(Type::List);
	ret.list = value;
	return ret;
}
inline Object ListObject(const ObjectList value)
{
	Object ret(Type::List);
	ret.list = new ObjectList;
	*ret.list = value;
	return ret;
}

// Must be down here, as it needs ObjectMap's definition
inline Object Object::operator[](const char* key) const
{
	return map->get(key);
}

// Convenience constructors
inline Object MapObject(ObjectMap* value)
{
	Object ret(Type::Map);
	ret.map = value;
	return ret;
}

}
