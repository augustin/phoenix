/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include "Object.h"

#include <string>

namespace Script {

// Predefinitions
class Stack;

Object Debugger(Stack* stack, std::string path);

}
