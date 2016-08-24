/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Target.h"

#include "script/Stack.h"
#include "script/Function.h"
#include "util/FSUtil.h"
#include "util/PrintUtil.h"
#include "util/StringUtil.h"

#include "Generators.h"
#include "LanguageInfo.h"

using std::vector;
using Script::Exception;
using Script::Function;
using Script::FunctionObject;
using Script::Object;
using Script::ObjectMap;
using Script::Stack;
using Script::Type;

std::vector<Target*> Target::targets;

Target::Target(const ObjectMap& params)
{
	ObjectMap* map = fMapObject = new ObjectMap;
	targets.push_back(this);

	NativeFunction_COERCE_OR_THROW("0", nm, Type::String);
	name = nm.asStringRaw();

	const Object obj = params.get("language");
	if (obj.type() == Type::List) {
		for (const Object* o : *obj.list)
			languages.push_back(o->asStringRaw());
	} else
		languages.push_back(obj.asStringRaw());

	// Prefetch all LanguageInfos
	for (std::string lang : languages)
		LanguageInfo::getLanguageInfo(lang);

	// TODO: get rid of hard-coded languages[0]
	otherFlags = LanguageInfo::getLanguageInfo(languages[0])->compilerDefaultFlags;

	map->set("setStandardsMode", FunctionObject([this](Stack*, Object*, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", modeNameObj, Type::String);

		bool strict = params.get("strict").boolean;
		std::string modeName = modeNameObj.asStringRaw();
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(this->languages[0]);
		if (StringUtil::startsWith(modeName, info->name))
			modeName.erase(0, info->name.length());
		if (info->standardsModes.count(modeName) == 0)
			throw Exception(Exception::UserError,
				std::string("standards mode '" + modeName + "' for language " +
					 info->name + " does not exist!"));
		if (info->checkStandardsMode(modeName)) {
			this->standardsModeFlag =
				strict ? info->standardsModes[modeName].strictFlag :
						 info->standardsModes[modeName].normalFlag;
		} else
			PrintUtil::warning("standards mode '" + modeName + "' for language "
				+ info->name + " unsupported!");
		return Object();
	}));

	map->set("addDefinitions", FunctionObject([this](Stack*, Object*, ObjectMap& m) -> Object {
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(this->languages[0]);

		for (ObjectMap::const_iterator it = m.begin(); it != m.end(); it++) {
			std::string name = it->first, val;
			if (name == "0")
				name = it->second->asStringRaw();
			else if (it->second->type() != Type::Boolean && it->second->boolean != true)
				val = it->second->asStringRaw();
			StringUtil::replaceAll(val, "\"", "\\\"");
			this->definitionsFlags.append(" \"" + info->compilerDefinition +
				name + (val.empty() ? "\"" : "=" + val + "\""));
		}
		return Object();
	}));

	map->set("addSources", FunctionObject([this](Stack* stack,
			Object*, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", filesObj, Type::List);
		for (const Object* o : *filesObj.list) {
			this->sourceFiles.push_back(FSUtil::absolutePath(FSUtil::combinePaths({
				stack->currentDir(), o->asStringRaw()})));
		}
		return Object();
	}));
	map->set("addSourceDirectory", FunctionObject([this](Stack* stack, Object*,
			ObjectMap& params) -> Object {
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(this->languages[0]);
		NativeFunction_COERCE_OR_THROW("0", dirNameObj, Type::String);
		std::string dirName = FSUtil::combinePaths({stack->currentDir(),
			dirNameObj.asStringRaw()});
		bool recurse = params.get("recursive").boolean;
		vector<std::string> newFiles =
			FSUtil::searchForFiles(dirName, info->sourceExtensions, recurse);
		for (vector<std::string>::size_type i = 0; i < newFiles.size(); i++)
			newFiles[i] = FSUtil::absolutePath(newFiles[i]);
		this->sourceFiles.insert(this->sourceFiles.end(), newFiles.begin(),
			newFiles.end());
		return Object();
	}));

	map->set("addIncludeDirectories", FunctionObject([this](Stack* stack,
			Object*, ObjectMap& params) -> Object {
		// TODO: get rid of hardcoded languages[0]
		LanguageInfo* info = LanguageInfo::getLanguageInfo(this->languages[0]);
		NativeFunction_COERCE_OR_THROW("0", dirsList, Type::List);

		for (const Object* itm : *dirsList.list) {
			this->includesFlags.append(" " + info->compilerInclude + "\"" +
				FSUtil::combinePaths({stack->currentDir(), itm->asStringRaw()}) + "\"");
		}
		return Object();
	}));
}

void Target::generate(Generator* gen)
{
	for (std::string lang : languages)
		LanguageInfo::getLanguageInfo(lang)->generate(gen);
	if (sourceFiles.size() == 0) {
		throw Exception(Exception::TypeError, "no source files for target '" + name + "'");
	}
	gen->addTarget(name + APPLICATION_FILE_EXT, sourceFiles,
		StringUtil::join({otherFlags, includesFlags,
			definitionsFlags, standardsModeFlag}, " "));
}

void Target::addGlobalFunction(Script::Stack* stack)
{
	stack->GlobalFunctions.insert({"CreateTarget", Function([](Stack*, Object*, ObjectMap& params)
		-> Script::Object {
		return Script::MapObject((new Target(params))->fMapObject);
	})});
}
