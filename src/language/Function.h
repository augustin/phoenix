/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include "ObjectMap.h"

#include <functional>
#include <string>

namespace Language {

// Utilities for native function declarations
#define NativeFunction_COERCE_OR_THROW(NAME, VARIABLE, TYPE) \
	Language::Object VARIABLE = params.get(NAME); \
	Language::CoerceOrThrow("parameter '" NAME "'", VARIABLE, TYPE)

typedef std::function<Object(Object, ObjectMap&)> NativeStdFunction;

class Function
{
public:
	Function();
	Function(NativeStdFunction nativeFunction);

	Object call(Object context, ObjectMap& args);

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
