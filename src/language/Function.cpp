/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "language/Function.h"

using std::function;
using std::string;

namespace Language {

Function::Function(std::function<Object(ObjectMap&)> nativeFunction)
	:
	fNativeFunction(nativeFunction),
	fIsNative(true)
{
}

Object Function::call(ObjectMap& args)
{
	if (fIsNative)
		return fNativeFunction(args);
	throw Exception(Exception::AccessViolation, "Attempted to call null function");
}

};
