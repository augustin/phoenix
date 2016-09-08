/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "script/Function.h"

#include "script/Interpreter.h"

using std::function;
using std::string;

namespace Script {

Function::Function(string function, string functionFile, uint32_t functionLine)
	:
	fIsNull(false),
	fIsNative(false),
	fFunction(function),
	fFunctionFile(functionFile),
	fFunctionLine(functionLine)
{
}
Function::Function(NativeStdFunction nativeFunction)
	:
	fIsNull(false),
	fNativeFunction(nativeFunction),
	fIsNative(true)
{
}

Object Function::call(Stack* stack, Object context, ObjectMap& args)
{
	if (fIsNull) {
		throw Exception(Exception::AccessViolation, "cannot call null function");
	}
	if (fIsNative)
		return fNativeFunction(stack, context, args);

	stack->push();
	if (context != nullptr)
		stack->set_ptr({"this"}, context);
	stack->set_ptr({"__arguments"}, MapObject(new ObjectMap(args)), true);
	for (ObjectMap::const_iterator it = args.begin(); it != args.end(); it++)
		stack->set_ptr({it->first}, it->second, true);
	Object ret = EvalString(stack, fFunction, fFunctionFile, fFunctionLine, false);
	stack->pop();
	return ret;
}

}
