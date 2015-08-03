/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include "MapObject.h"

#include <functional>

namespace Language {

class FunctionObject : public Object
{
public:
	FunctionObject(std::function<Object(const MapObject&)> nativeFunction);

	Object call(const MapObject& args);

private:
	std::function<Object(const MapObject&)> fNativeFunction;
	bool fIsNative;
};

};
