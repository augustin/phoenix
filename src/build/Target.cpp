/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Target.h"

#include "language/Stack.h"
#include "language/Function.h"
#include "util/FSUtil.h"
#include "util/PrintUtil.h"
#include "util/StringUtil.h"

#include "Generators.h"
#include "LanguageInfo.h"

using std::vector;
using Language::Exception;
using Language::Function;
using Language::FunctionObject;
using Language::Object;
using Language::ObjectMap;
using Language::Stack;
using Language::Type;

namespace Target {
std::vector<ExtraData*> targets;

Object CreateTarget(const ObjectMap& params)
{
	Object ret = Language::MapObject(new ObjectMap);
	ExtraData* extraData = new Target::ExtraData;
	ret.extradata = extraData;
	targets.push_back(extraData);

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

	extraData->otherFlags = LanguageInfo::getLanguageInfo(extraData->languages[0])->compilerDefaultFlags;

	ret.map->set("setStandardsMode", FunctionObject([](Stack*, Object* self, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", modeNameObj, Type::String);
		ExtraData* extraData = static_cast<ExtraData*>(self->extradata);

		bool strict = params.get("strict").boolean;
		std::string modeName = modeNameObj.asStringRaw();
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(extraData->languages[0]);
		if (StringUtil::startsWith(modeName, info->name))
			modeName.erase(0, info->name.length());
		if (info->standardsModes.count(modeName) == 0)
			throw Exception(Exception::UserError,
				std::string("standards mode '" + modeName + "' for language " +
					 info->name + " does not exist!"));
		if (info->checkStandardsMode(modeName)) {
			extraData->standardsModeFlag =
				strict ? info->standardsModes[modeName].strictFlag :
						 info->standardsModes[modeName].normalFlag;
		} else
			PrintUtil::warning("standards mode '" + modeName + "' for language "
				+ info->name + " unsupported!");
		return Object();
	}));

	ret.map->set("addDefinitions", FunctionObject([](Stack*, Object* self, ObjectMap& m) -> Object {
		ExtraData* extraData = static_cast<ExtraData*>(self->extradata);
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

	ret.map->set("addSources", FunctionObject([](Stack* stack,
			Object* self, ObjectMap& params) -> Object {
		ExtraData* extraData = static_cast<ExtraData*>(self->extradata);
		NativeFunction_COERCE_OR_THROW("0", filesObj, Type::List);
		for (Object o : *filesObj.list) {
			extraData->sourceFiles.push_back(FSUtil::combinePaths({
				stack->currentDir(), o.asStringRaw()}));
		}
		return Object();
	}));
	ret.map->set("addSourceDirectory", FunctionObject([](Stack* stack, Object* self,
			ObjectMap& params) -> Object {
		ExtraData* extraData = static_cast<ExtraData*>(self->extradata);
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(extraData->languages[0]);
		NativeFunction_COERCE_OR_THROW("0", dirNameObj, Type::String);
		std::string dirName = FSUtil::combinePaths({stack->currentDir(),
			dirNameObj.asStringRaw()});
		bool recurse = params.get("recursive").boolean;
		vector<std::string> newFiles =
			FSUtil::searchForFiles(dirName, info->sourceExtensions, recurse);
		extraData->sourceFiles.insert(extraData->sourceFiles.end(), newFiles.begin(),
			newFiles.end());
		return Object();
	}));

	ret.map->set("addIncludeDirectories", FunctionObject([](Stack* stack,
			Object* self, ObjectMap& params) -> Object {
		ExtraData* extraData = static_cast<ExtraData*>(self->extradata);
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(extraData->languages[0]);
		NativeFunction_COERCE_OR_THROW("0", dirsList, Type::List);

		for (Object itm : *dirsList.list) {
			extraData->includesFlags.append(" \"" + info->compilerInclude +
				FSUtil::combinePaths({stack->currentDir(), itm.asStringRaw()}) + "\"");
		}
		return Object();
	}));

	return ret;
}

void generate(ExtraData* target, Generator* gen)
{
	for (std::string lang : target->languages)
		LanguageInfo::getLanguageInfo(lang)->generate(gen);
	gen->addTarget(target->name, target->sourceFiles,
		StringUtil::join({target->otherFlags, target->includesFlags,
			target->definitionsFlags, target->standardsModeFlag}, " "));
}

void addGlobalFunction()
{
	Language::GlobalFunctions.insert({"CreateTarget", Function([](Stack*, Object*, ObjectMap& params)
		-> Language::Object {
		return CreateTarget(params);
	})});
}

}
