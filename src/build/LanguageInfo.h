/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>

#include "language/Object.h"
#include "language/Stack.h"

class LanguageInfo
{
public:
	static Language::Object* getLanguageInfo(std::string langName);
	static Language::Stack* sStack;

private:
	static Language::ObjectMap sData;
};
