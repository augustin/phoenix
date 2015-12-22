/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "LanguageInfo.h"

#include <string>

#include "language/Parser.h"
#include "util/FSUtil.h"
#include "util/PrintUtil.h"
#include "util/OSUtil.h"
#include "util/StringUtil.h"

using std::string;
using Language::Object;
using Language::Type;

std::map<string, string> LanguageInfo::sPreferredCompiler;
std::map<string, LanguageInfo*> LanguageInfo::sData;

LanguageInfo::LanguageInfo(string langName, Object info)
	: name(langName)
{
	Language::CoerceOrThrow("language information", info, Type::Map);
	Object type = info.map->get("type");
	Language::CoerceOrThrow("languageInfo.type", type, Type::String);
	if (type.string != "CompiledToMachineCode")
		throw Language::Exception(Language::Exception::Type::UserError,
			"only 'CompiledToMachineCode' is supported at this time");

	// File extensions
	Object srcExts = info.map->get("sourceExtensions");
	Language::CoerceOrThrow("languageInfo.sourceExtensions", srcExts, Type::List);
	for (const Object& obj : *srcExts.list)
		sourceExtensions.push_back(obj.asStringRaw());
	Object extraExts = info.map->get("extraExtensions");
	if (extraExts.type() == Language::Type::List) {
		for (const Object& obj : *extraExts.list)
			extraExtensions.push_back(obj.asStringRaw());
	}

	preprocessor = info.map->get("preprocessor").boolean;

	Object envir = info.map->get("compilerEnviron");
	if (envir.type() != Language::Type::Undefined)
		compilerEnviron = envir.asStringRaw();

	// Figure out what compiler we're using
	Object comps = info.map->get("compilers");
	Language::CoerceOrThrow("languageInfo.compilers", comps, Type::Map);
	auto tryCompiler = [&](const string& binary) -> bool {
		string compilerBin = FSUtil::which(binary);
		if (FSUtil::exists(compilerBin)) {
			// Which compiler is this?
			for (Language::ObjectMap::const_iterator it =
				 comps.map->begin(); it != comps.map->end(); it++) {
				Object* comp = it->second;
				Language::CoerceOrThrowPtr("languageInfo.compiler", comp, Type::Map);
				Object detect = comp->map->get("detect");
				Language::CoerceOrThrow("languageInfo.compiler.detect", detect, Type::Map);
				OSUtil::ExecResult res =
					OSUtil::exec(binary + " " + detect.map->get("arguments").asStringRaw());
				if (res.exitcode != 0)
					continue; // did not exit with 0 -- something wrong
				Object contains = detect.map->get("contains");
				Language::CoerceOrThrow("languageInfo.compiler.detect.contains", contains, Type::List);
				bool OK = true;
				for (const Object& obj : *contains.list) {
					if (OK && res.output.find(obj.asStringRaw()) == string::npos)
						OK = false;
				}
				if (OK) {
					compilerName = it->first;
					compilerBinary = compilerBin;
					compilerCompileFlag = comp->map->get("compile").asStringRaw();
					compilerOutputFlag = comp->map->get("output").asStringRaw();
					compilerDefinition = comp->map->get("definition").asStringRaw();
					compilerInclude = comp->map->get("include").asStringRaw();
					return true;
				}
			}
		}
		return false;
	};

	PrintUtil::checking("what " + langName + " compiler to use");
	if (!sPreferredCompiler[langName].empty()) {
		tryCompiler(sPreferredCompiler[langName]);
	}
	if (compilerName.empty() && !compilerEnviron.empty()) {
		const char* env = getenv(compilerEnviron.c_str());
		if (env != nullptr)
			tryCompiler(env);
	}
	if (compilerName.empty()) {
		// Preferred & environ didn't work, try everything in succession instead
		for (Language::ObjectMap::const_iterator it =
			 comps.map->begin(); it != comps.map->end(); it++) {
			Object* comp = it->second;
			Language::CoerceOrThrowPtr("languageInfo.compiler", comp, Type::Map);
			if (tryCompiler(comp->map->get("binary").asStringRaw()))
				break;
		}
	}
	if (compilerName.empty()) {
		PrintUtil::checkFinished("none found", 0);
		throw Language::Exception(Language::Exception::UserError,
			string("cannot find a compiler for " + langName));
	} else
		PrintUtil::checkFinished(compilerName + " ('" + compilerBinary + "')", 2);

	// Check that the compiler works
	PrintUtil::checking("if the " + langName + " compiler works");
	FSUtil::mkdir("PhoenixFiles");
	string testFile = "PhoenixFiles/test" + langName + sourceExtensions[0];
	FSUtil::putContents(testFile, info.map->get("test").asStringRaw());
	OSUtil::ExecResult res = OSUtil::exec(compilerBinary +
		" " + testFile + " " + compilerOutputFlag + testFile + OBJECT_FILE_EXT);
	if (res.exitcode == 0)
		PrintUtil::checkFinished("yes", 2);
	else {
		PrintUtil::checkFinished("no", 0);
		throw Language::Exception(Language::Exception::UserError,
			string("complier for " + langName + " is broken: '" + res.output + "'"));
	}
}

LanguageInfo* LanguageInfo::getLanguageInfo(string langName)
{
	LanguageInfo* ret = sData[langName];
	if (ret != nullptr)
		return ret;

	// FIXME/TODO: this is bootstrap-only Phoenix, load hardcoded location
	Language::Stack stack;
	Object info = Language::Run(&stack,
		FSUtil::combinePaths({FSUtil::parentDirectory(__FILE__), "../data/languages/" + langName + ".phnx"}));
	LanguageInfo* langInfo = new LanguageInfo(langName, info);
	sData.insert({langName, langInfo});
	return langInfo;
}
