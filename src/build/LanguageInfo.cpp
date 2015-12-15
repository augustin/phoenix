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

std::map<string, LanguageInfo*> LanguageInfo::sData;

LanguageInfo::LanguageInfo(string langName, Object info)
	: fName(langName)
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
		fSourceExtensions.push_back(obj.asStringRaw());
	Object extraExts = info.map->get("extraExtensions");
	if (extraExts.type() == Language::Type::List) {
		for (const Object& obj : *extraExts.list)
			fExtraExtensions.push_back(obj.asStringRaw());
	}

	fPreprocessor = info.map->get("preprocessor").boolean;

	Object envir = info.map->get("compilerEnviron");
	if (envir.type() != Language::Type::Undefined)
		fCompilerEnviron = envir.asStringRaw();

	// Figure out what compiler we're using
	Object comps = info.map->get("compilers");
	Language_COERCE_OR_THROW("languageInfo.compilers", comps, Map);
	auto tryCompiler = [&](const string& binary) -> bool {
		string compilerBinary = FSUtil::which(binary);
		if (FSUtil::exists(compilerBinary)) {
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
					fCompilerName = it->first;
					fCompilerBinary = compilerBinary;
					fCompilerCompile = comp->map->get("compile").asStringRaw();
					fCompilerDefinition = comp->map->get("definition").asStringRaw();
					fCompilerInclude = comp->map->get("include").asStringRaw();
					return true;
				}
			}
		}
		return false;
	};

	PrintUtil::checking("what " + langName + " compiler to use");
	string compilerBinary;
	if (!fCompilerEnviron.empty()) {
		const char* env = getenv(fCompilerEnviron.c_str());
		if (env != nullptr)
			compilerBinary = string(env);
	}
	if (!tryCompiler(compilerBinary)) {
		// Environ didn't work, try everything in succession instead
		for (Language::ObjectMap::const_iterator it =
			 comps.map->begin(); it != comps.map->end(); it++) {
			Object* comp = it->second;
			Language_COERCE_OR_THROW_PTR("languageInfo.compiler", comp, Map);
			if (tryCompiler(comp->map->get("binary").asStringRaw()))
				break;
		}
	}
	if (fCompilerName.empty()) {
		PrintUtil::checkFinished("none found", 0);
		throw Language::Exception(Language::Exception::UserError,
			string("cannot find a compiler for " + langName));
	} else
		PrintUtil::checkFinished(fCompilerName + " ('" + fCompilerBinary + "')", 2);
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
