/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace Language {

class Exception final : public std::exception
{
public:
	enum Type {
		FileDoesNotExist = 2, // number here is return code, 1 is generic
		SyntaxError,
		AccessViolation
	};
	Exception(Type type, std::string what) : fWhat(what), fType(type) {}
	virtual const char* what() const throw() { return fWhat.c_str(); }
	inline Type type() { return fType; }

	void print() {
		std::cout << "error: ";
		switch (fType) {
		case FileDoesNotExist:
			std::cout << "file '" << fWhat << "' does not exist.";
		break;
		default:
			std::cout << "unknown error " << fType << ": '" << fWhat << "'.";
		break;
		}
	}

private:
	std::string fWhat;
	Type fType;
};


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
