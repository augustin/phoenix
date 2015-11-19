/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "ObjectMap.h"

using std::map;
using std::string;

namespace Language {

ObjectMap::ObjectMap()
	:
	map<string, Object>()
{
}

Object ObjectMap::get(string key, bool returnNonexistent)
{
	map<string, Object>::iterator it = find(key);
	if (it != end())
		return it->second;
	if (returnNonexistent)
		return Object(Type::Nonexistent);
	return Object(Type::Undefined);
}

void ObjectMap::set(string key, Object value)
{
	map<string, Object>::iterator it = find(key);
	if (it != end())
		it->second = value;
	else
		insert({key, value});
}

}
