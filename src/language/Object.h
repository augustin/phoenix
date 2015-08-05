/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>
#include <vector>

namespace Language {

enum Type {
	Undefined = 0,
	Boolean,
	Integer,
	String,
	Function,
	List,
	Map
};

class Object
{
public:
	explicit Object();

	inline Type type() { return fObjectType; }
	std::string typeName();

protected:
	Object(const Type type);

private:
	Type fObjectType;
};

class BooleanObject : public Object
{
public:
	BooleanObject(const bool val = false);
	bool value;
};

class IntegerObject : public Object
{
public:
	IntegerObject(const int val = 0);
	int value;
};

class StringObject : public Object, public std::string
{
public:
	StringObject(const std::string& str = "");
};

};
