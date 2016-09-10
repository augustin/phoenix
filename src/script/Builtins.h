/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include "Object.h"

namespace Script {

// Predefinitions
class GlobalPhoenixObject;
class Stack;

class FileBuiltin
{
public:
	FileBuiltin(const std::string& file);

private:
	friend class GlobalPhoenixObject;

	Script::ObjectMap* fMap;
	std::string fFile;
};

class GlobalPhoenixObject : public CObject
{
public:
	GlobalPhoenixObject(Stack* stack);
};

}
