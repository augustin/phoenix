/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>

#include "Object.h"

namespace Language {

class ObjectMap : public std::map<std::string, Object>
{
public:
	ObjectMap();

	Object get(std::string key, bool returnNonexistent = false);
	void set(std::string key, Object value);
};

// Convenience constructors
inline Object MapObject(ObjectMap* value)
{
	Object ret(Type::Map);
	ret.map = value;
	return ret;
}

}
