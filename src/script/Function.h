/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include "Object.h"

#include <functional>
#include <string>

namespace Script {

// Predefinitions
class Stack;

// Utilities for native function declarations
#define NativeFunction_COERCE_OR_THROW(NAME, VARIABLE, TYPE) \
	Script::Object VARIABLE = params.get(NAME); \
	Script::CoerceOrThrow("parameter '" NAME "'", VARIABLE, TYPE)

typedef std::function<Object(Stack*, Object*, ObjectMap&)> NativeStdFunction;

class Function
{
public:
	Function();
	Function(NativeStdFunction nativeFunction);

	Object call(Stack* stack, Object* context, ObjectMap& args);

private:
	NativeStdFunction fNativeFunction;
	bool fIsNative;
};

// Convenience constructors
inline Object FunctionObject(NativeStdFunction nativeFunction)
{
	Object ret(Type::Function);
	ret.function = new Function(nativeFunction);
	return ret;
}
inline Object FunctionObject(Function* value)
{
	Object ret(Type::Function);
	ret.function = value;
	return ret;
}

}
