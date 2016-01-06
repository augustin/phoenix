/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Target.h"

#include "language/Stack.h"
#include "language/Function.h"
#include "util/PrintUtil.h"
#include "util/StringUtil.h"

#include "LanguageInfo.h"

using Language::Exception;
using Language::Function;
using Language::FunctionObject;
using Language::Object;
using Language::ObjectMap;
using Language::Type;

namespace Target {

Object CreateTarget(const ObjectMap& params)
{
	Object ret = Language::MapObject(new ObjectMap);
	ExtraData* extraData = new Target::ExtraData;
	ret.extradata = extraData;

	NativeFunction_COERCE_OR_THROW("0", name, Type::String);
	extraData->name = name.asStringRaw();

	const Object obj = params.get("language");
	if (obj.type() == Type::List) {
		for (const Object& o : *obj.list)
			extraData->languages.push_back(o.asStringRaw());
	} else
		extraData->languages.push_back(obj.asStringRaw());

	// Prefetch all LanguageInfos
	for (std::string lang : extraData->languages)
		LanguageInfo::getLanguageInfo(lang);

	ret.map->set("setStandardsMode", FunctionObject([](Object self, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", modeNameObj, Type::String);
		ExtraData* extraData = static_cast<ExtraData*>(self.extradata);

		bool strict = params.get("strict").boolean;
		std::string modeName = modeNameObj.asStringRaw();
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(extraData->languages[0]);
		if (StringUtil::startsWith(modeName, info->name))
			modeName.erase(0, info->name.length());
		if (info->standardsModes.count(modeName) == 0)
			throw Exception(Exception::UserError,
				std::string("standards mode '" + modeName + "' for language " + info->name + " does not exist!"));
		if (info->checkStandardsMode(modeName)) {
			extraData->standardsModeFlag =
				strict ? info->standardsModes[modeName].strictFlag :
						 info->standardsModes[modeName].normalFlag;
		} else
			PrintUtil::warning("standards mode '" + modeName + "' for language " + info->name + " unsupported");
		return Object();
	}));

	ret.map->set("addDefinitions", FunctionObject([](Object self, ObjectMap& m) -> Object {
		ExtraData* extraData = static_cast<ExtraData*>(self.extradata);
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(extraData->languages[0]);

		for (ObjectMap::const_iterator it = m.begin(); it != m.end(); it++) {
			std::string val = it->second->asStringRaw();
			StringUtil::replaceAll(val, "\"", "\\\"");
			extraData->definitionsFlags.append(" \"" + info->compilerDefinition +
				it->first + "=" + val + "\"");
		}
		return Object();
	}));

	return ret;
}

void addGlobalFunction()
{
	Language::GlobalFunctions.insert({"CreateTarget", Function([](Object, ObjectMap& params)
		-> Language::Object {
		return CreateTarget(params);
	})});
}

}
