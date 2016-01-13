/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "language/Function.h"

using std::function;
using std::string;

namespace Language {

Function::Function()
	:
	fIsNative(false)
{
}
Function::Function(NativeStdFunction nativeFunction)
	:
	fNativeFunction(nativeFunction),
	fIsNative(true)
{
}

Object Function::call(Stack* stack, Object* context, ObjectMap& args)
{
	if (fIsNative)
		return fNativeFunction(stack, context, args);
	throw Exception(Exception::AccessViolation, "Attempted to call null function");
}

}
