/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "GlobalLanguageObject.h"

#include "FunctionObject.h"

using std::string;

namespace Language {

GlobalLanguageObject::GlobalLanguageObject()
	:
	MapObject()
{
	add("checkVersion", FunctionObject([&](const MapObject& params) -> Object {
		return BooleanObject(false);
	}));
}

};
