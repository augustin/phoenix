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
	Function() : fIsNull(true) {}
	Function(std::string function, std::string functionFile, uint32_t functionLine);
	Function(NativeStdFunction nativeFunction);

	Object call(Stack* stack, Object* context, ObjectMap& args);

	bool isNative() const { return fIsNative; }

private:
	bool fIsNull;
	NativeStdFunction fNativeFunction;
	bool fIsNative;

	std::string fFunction;
	std::string fFunctionFile;
	uint32_t fFunctionLine;
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
