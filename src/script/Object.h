/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <exception>
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace Script {

// Predefinitions
class Function;
class ObjectMap;
class ObjectList;
class CObject;
typedef std::shared_ptr<CObject> Object;

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

class CObject
{
public:
	CObject(const Type type = Type::Undefined);
	~CObject();
	// Copy constructors
	CObject(const CObject& other);
	CObject& operator=(const CObject& other);

	inline Type type() const { return fType; }
	static std::string typeName(Type type);
	std::string typeName() const;
	std::string asStringPretty() const;
	std::string asStringRaw() const;

	inline Object get(const char* key) const;

	// Helpers
	bool coerceToBoolean() const;
	// Operators
	static Object op_div(const Object left, const Object right);
	static Object op_mult(const Object left, const Object right);
	static Object op_subt(const Object left, const Object right);
	static Object op_add(const Object left, const Object right);
	static Object op_eq(const Object left, const Object right);
	inline static Object op_neq(const Object left, const Object right)
		{ Object o = op_eq(left, right); o->boolean = !o->boolean; return o; }
	static Object op_and(const Object left, const Object right);
	static Object op_or(const Object left, const Object right);
	static Object op_lt(const Object left, const Object right);
	static Object op_gt(const Object left, const Object right);
	static Object op_lteq(const Object left, const Object right);
	static Object op_gteq(const Object left, const Object right);

public: // Data storage
	bool boolean;
	int32_t integer;
	std::string string;
	Function* function;
	ObjectList* list;
	ObjectMap* map;

private:
	Type fType;
};

// Helper functions
inline void CoerceOrThrow(const std::string& what, const Object variable, Type type)
{
	if (variable == nullptr) {
		throw Exception(Exception::TypeError,
			std::string(what).append(" should be of type '").append(CObject::typeName(type))
				.append("' but is of type 'undefined'"));
	}
	if (variable->type() != type) {
		throw Exception(Exception::TypeError,
			std::string(what).append(" should be of type '")
				.append(CObject::typeName(type)).append("' but is of type '")
				.append(variable->typeName()).append("'"));
	}
}

// Convenience constructors
inline Object UndefinedObject()
{
	Object ret = std::make_shared<CObject>(Type::Undefined);
	return ret;
}
inline Object BooleanObject(const bool value)
{
	Object ret = std::make_shared<CObject>(Type::Boolean);
	ret->boolean = value;
	return ret;
}
inline Object IntegerObject(const int value)
{
	Object ret = std::make_shared<CObject>(Type::Integer);
	ret->integer = value;
	return ret;
}
inline Object StringObject(const std::string& value)
{
	Object ret = std::make_shared<CObject>(Type::String);
	ret->string = std::string(value);
	return ret;
}
inline Object CopyObject(const Object other)
{
	return std::make_shared<CObject>(*other.get());
}

// ObjectMap (defined here, implemented in ObjectMap.cpp)
class ObjectMap : private std::map<std::string, Object>
{
	typedef std::map<std::string, Object> _inherited;
public:
	ObjectMap();
	~ObjectMap();

	typedef _inherited::const_iterator const_iterator;
	const_iterator begin() const { return _inherited::begin(); }
	const_iterator end() const { return _inherited::end(); }

	Object get(std::string key) const;
	Object get_ptr(std::string key);
	void set_ptr(std::string key, Object value);
	inline void set(std::string key, Object value) { set_ptr(key, CopyObject(value)); }

	size_type size() const { return _inherited::size(); }
};

class ObjectList : private std::vector<Object>
{
	typedef std::vector<Object> _inherited;
public:
	ObjectList() {}
	~ObjectList() {}

	typedef _inherited::const_iterator const_iterator;
	typedef _inherited::size_type size_type;
	const_iterator begin() const { return _inherited::begin(); }
	const_iterator end() const { return _inherited::end(); }

	void push_back(const Object obj) { _inherited::push_back(CopyObject(obj)); }

	Object get(_inherited::size_type i) { return CopyObject(get_ptr(i)); }
	Object get_ptr(_inherited::size_type i) { return _inherited::at(i); }

	void set(_inherited::size_type i, const Object obj) { set_ptr(i, CopyObject(obj)); }
	void set_ptr(_inherited::size_type i, const Object obj) { _inherited::operator[](i) = obj; }

	size_type size() const { return _inherited::size(); }
};

// Must be down here, as it needs ObjectList's definition
inline Object ListObject(ObjectList* value)
{
	Object ret = std::make_shared<CObject>(Type::List);
	ret->list = value;
	return ret;
}

// Must be down here, as it needs ObjectMap's definition
inline Object CObject::get(const char* key) const
{
	return map->get(key);
}
inline Object MapObject(ObjectMap* value)
{
	Object ret = std::make_shared<CObject>(Type::Map);
	ret->map = value;
	return ret;
}

}
