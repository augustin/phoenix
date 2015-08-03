/*
 * (C) 2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "language/FunctionObject.h"

using std::function;
using std::string;

namespace Language {

FunctionObject::FunctionObject(function<Object(const MapObject&)> nativeFunction)
	:
	Object(Type::Function),
	fNativeFunction(nativeFunction),
	fIsNative(true)
{
}

Object FunctionObject::call(const MapObject& args)
{
	// TODO: Parameter type checking.
	if (fIsNative)
		return fNativeFunction(args);
	return Object();
}

};
