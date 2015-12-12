/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Target.h"

#include "language/Stack.h"

#include "LanguageInfo.h"

#include <string>
#include <vector>
#include <iostream>

using Language::Function;
using Language::ObjectMap;
using Language::Exception;

Target::Target(const ObjectMap& params)
{
	NativeFunction_COERCE_OR_THROW("0", name, String);
	std::vector<std::string> languages;
	const Object obj = params.get("language");
	if (obj.type() == Language::Type::String) {
		languages.push_back(obj.string);
	} else { // TODO: type=list for multilingual programs
		throw Exception(Language::Exception::TypeError,
			std::string("parameter 'name' should be of type 'String' but is of type '")
				.append(obj.typeName()).append("'"));
	}

	for (std::string lang : languages) {
		Object* info = LanguageInfo::getLanguageInfo(lang);
		std::cout << info->string;
	}
}

void Target::addGlobalFunction()
{
	Language::GlobalFunctions.insert({"CreateTarget", Function([](ObjectMap& params)
		-> Language::Object {
		return Target(params);
	})});
}
