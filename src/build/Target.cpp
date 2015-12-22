/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Target.h"

#include "language/Stack.h"

#include "LanguageInfo.h"

#include <string>
#include <vector>

using Language::Function;
using Language::ObjectMap;
using Language::Exception;

Target::Target(const ObjectMap& params)
{
	NativeFunction_COERCE_OR_THROW("0", name, Language::Type::String);
	std::vector<std::string> languages;
	const Object obj = params.get("language");
	if (obj.type() == Language::Type::List) {
		for (const Object& o : *obj.list)
			languages.push_back(o.asStringRaw());
	} else
		languages.push_back(obj.asStringRaw());

	for (std::string lang : languages) {
		LanguageInfo* info = LanguageInfo::getLanguageInfo(lang);
		//std::cout << info->string;
	}
}

void Target::addGlobalFunction()
{
	Language::GlobalFunctions.insert({"CreateTarget", Function([](ObjectMap& params)
		-> Language::Object {
		return Target(params);
	})});
}
