/*
 * (C) 2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>

#include "Object.h"

namespace Language {

class MapObject : public Object, public std::map<std::string, Object>
{
public:
	MapObject();

	void add(std::string key, Object value);
};

};
