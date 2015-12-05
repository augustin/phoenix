/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include "ObjectMap.h"

#include <functional>
#include <string>

namespace Language {

// Utilities for native function declarations
#define NativeFunction_COERCE_OR_THROW(NAME, VARIABLE, TYPE) \
	Object VARIABLE = params.get(NAME); \
	Language_COERCE_OR_THROW("parameter '" NAME "'", VARIABLE, TYPE)

class Function
{
public:
	Function();
	Function(std::function<Object(ObjectMap&)> nativeFunction);

	Object call(ObjectMap& args);

private:
	std::function<Object(ObjectMap&)> fNativeFunction;
	bool fIsNative;
};

// Convenience constructors
inline Object FunctionObject(std::function<Object(ObjectMap&)> nativeFunction)
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
