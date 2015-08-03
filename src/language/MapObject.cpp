/*
 * (C) 2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "MapObject.h"

using std::map;
using std::string;

namespace Language {

MapObject::MapObject()
	:
	Object(Type::Map),
	map<string, Object>()
{
}

void MapObject::add(string key, Object value)
{
	map<string, Object>::insert(std::pair<string, Object>(key, value));
}

};
