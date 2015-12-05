/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "GlobalLanguageObject.h"

#include <iostream>
#include <vector>

#include "Phoenix.h"

#include "Function.h"
#include "ObjectMap.h"
#include "Stack.h"
#include "util/StringUtil.h"

using std::vector;

namespace Language {

GlobalLanguageObject::GlobalLanguageObject()
	:
	Object(Type::Map)
{
	map = new ObjectMap;

	// Instantiate this object
	map->set("checkVersion", FunctionObject([&](ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("minimum", minimum, String);
		vector<std::string> components = StringUtil::split(minimum.string, ".");
		if (components.size() == 0)
			throw Exception(Exception::TypeError,
				std::string("'minimum' must have at least 1 component"));
		vector<int> parts;
		for (const std::string& str : components)
			parts.push_back(strtol(str.c_str(), NULL, 10));
		while (parts.size() != 3)
			parts.push_back(-1);
		if (parts[0] > PHOENIX_VERSION_MAJOR ||
			parts[1] > PHOENIX_VERSION_MINOR ||
			parts[2] > PHOENIX_VERSION_PATCH)
			throw Exception(Exception::UserError,
				std::string("minimum version required is ").append(minimum.string)
				 .append(" and this is Phoenix " PHOENIX_VERSION));
		return Object();
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
