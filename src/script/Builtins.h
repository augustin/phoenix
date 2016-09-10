/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include "Object.h"

namespace Script {

// Predefinitions
class Stack;

class GlobalPhoenixObject : public CObject
{
public:
	GlobalPhoenixObject(Stack* stack);
};

}
