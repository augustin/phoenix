/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Parser.h"

#include "util/FSUtil.h"
#include "util/StringUtil.h"
#include "Object.h"
#include "Stack.h"

using std::string;

namespace Language {

void Parse(Stack* stack, string path)
{
	string filename;
	if (StringUtil::endsWith(path, "Phoenixfile")) {
		filename = path;
	} else if (FSUtil::exists(path = FSUtil::combinePaths({path, "Phoenixfile"}))) {
		filename = path;
	}
	if (filename.empty())
		throw Exception(Exception::FileDoesNotExist, path);
}

};
