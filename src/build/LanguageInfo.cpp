/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "LanguageInfo.h"

#include <string>

#include "language/Parser.h"
#include "util/PrintUtil.h"
#include "util/ProcessUtil.h"
#include "util/FSUtil.h"

using std::string;
using Language::Object;

std::map<string, string> LanguageInfo::sPreferredCompiler;
std::map<string, LanguageInfo*> LanguageInfo::sData;

LanguageInfo::LanguageInfo(string langName, Object info)
	: name(langName)
{
	Language_COERCE_OR_THROW("language information", info, Map);
	Object type = info.map->get("type");
	Language_COERCE_OR_THROW("languageInfo.type", type, String);
	if (type.string != "CompiledToMachineCode")
		throw Language::Exception(Language::Exception::Type::UserError,
			"only 'CompiledToMachineCode' is supported at this time");

	// File extensions
	Object srcExts = info.map->get("sourceExtensions");
	Language_COERCE_OR_THROW("languageInfo.sourceExtensions", srcExts, List);
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
	Language_COERCE_OR_THROW("languageInfo.compilers", comps, Map);
	auto tryCompiler = [&](const string& binary) -> bool {
		string compilerBin = FSUtil::which(binary);
		if (FSUtil::exists(compilerBin)) {
			// Which compiler is this?
			for (Language::ObjectMap::const_iterator it =
				 comps.map->begin(); it != comps.map->end(); it++) {
				Object* comp = it->second;
				Language_COERCE_OR_THROW_PTR("languageInfo.compiler", comp, Map);
				Object detect = comp->map->get("detect");
				Language_COERCE_OR_THROW("languageInfo.compiler.detect", detect, Map);
				ProcessUtil::ExecResult res =
					ProcessUtil::exec(binary + " " + detect.map->get("arguments").asStringRaw());
				if (res.exitcode != 0)
					continue; // did not exit with 0 -- something wrong
				Object contains = detect.map->get("contains");
				Language_COERCE_OR_THROW("languageInfo.compiler.detect.contains", contains, List);
				bool OK = true;
				for (const Object& obj : *contains.list) {
					if (OK && res.output.find(obj.asStringRaw()) == string::npos)
						OK = false;
				}
				if (OK) {
					compilerName = it->first;
					compilerBinary = compilerBin;
					compilerCompile = comp->map->get("compile").asStringRaw();
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
			tryCompiler(string(env));
	}
	if (compilerName.empty()) {
		// Preferred & environ didn't work, try everything in succession instead
		for (Language::ObjectMap::const_iterator it =
			 comps.map->begin(); it != comps.map->end(); it++) {
			Object* comp = it->second;
			Language_COERCE_OR_THROW_PTR("languageInfo.compiler", comp, Map);
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
}

LanguageInfo* LanguageInfo::getLanguageInfo(string langName)
{
	LanguageInfo* ret = sData[langName];
	if (ret != nullptr)
		return ret;

	// FIXME/TODO: this is bootstrap-only Phoenix, load hardcoded location
	Language::Stack* stack = new Language::Stack;
	Object info = Language::Run(stack, "src/data/languages/" + langName + ".phnx");
	delete stack;
	LanguageInfo* langInfo = new LanguageInfo(langName, info);
	sData.insert({langName, langInfo});
	return langInfo;
}
