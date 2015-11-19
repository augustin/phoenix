/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "GlobalLanguageObject.h"

#include <iostream>

#include "Function.h"
#include "ObjectMap.h"
#include "Stack.h"

using std::string;

namespace Language {

GlobalLanguageObject::GlobalLanguageObject()
	:
	Object(Type::Map)
{
	map = new ObjectMap;

	// Instantiate this object
	map->set("checkVersion", FunctionObject([&](ObjectMap& params) -> Object {
		return BooleanObject(false);
	}));

	// Also instantiate the GlobalFunctions object
	GlobalFunctions.insert({"print", Function([](ObjectMap& params) -> Object {
		Object zero = params.get("0");
		if (zero.type() == Type::String)
			std::cout << "message: " << zero.string << "\n";
		else
			std::cout << "message: " << zero.asString() << "\n";
		return Object();
	})});
	GlobalFunctions.insert({"fatal", Function([](ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", zero, String);
		throw Exception(Exception::UserError, std::string(zero.string));
	})});
}

}
