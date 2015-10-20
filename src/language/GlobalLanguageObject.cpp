/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "GlobalLanguageObject.h"

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
	GlobalFunctions.insert({"funky_call", Function([&](ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("some_param_thing", someParamThing, String);
		return Object();
	})});
}

}
