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

Object ObjectMap::get(string key)
{
	map<string, Object>::iterator it = find(key);
	if (it != end())
		return it->second;
	return Object(Type::Nonexistent);
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
