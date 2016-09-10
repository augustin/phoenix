/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "LanguageInfo.h"

#include <string>

#include "script/Interpreter.h"
#include "util/FSUtil.h"
#include "util/PrintUtil.h"
#include "util/StringUtil.h"

using std::string;
using Script::Object;
using Script::Type;

// Statics
Script::Stack* LanguageInfo::sStack;
std::map<string, string> LanguageInfo::sPreferredCompiler;
std::map<string, LanguageInfo*> LanguageInfo::sData;

LanguageInfo::LanguageInfo(string langName, Object info)
	: name(langName),
	  fGenerated(false)
{
	genName = name;
	StringUtil::replaceAll(genName, "+", "P");
	StringUtil::replaceAll(genName, "-", "D");

	Script::CoerceOrThrow("language information", info, Type::Map);
	Object type = info->get("type");
	Script::CoerceOrThrow("languageInfo.type", type, Type::String);
	if (type->string != "CompiledToMachineCode")
		throw Script::Exception(Script::Exception::Type::UserError,
			"only 'CompiledToMachineCode' is supported at this time");

	// File extensions
	Object srcExts = info->get("sourceExtensions");
	Script::CoerceOrThrow("languageInfo.sourceExtensions", srcExts, Type::List);
	for (const Object obj : *srcExts->list)
		sourceExtensions.push_back(obj->asStringRaw());
	Object extraExts = info->get("extraExtensions");
	if (extraExts->type() == Script::Type::List) {
		for (const Object obj : *extraExts->list)
			extraExtensions.push_back(obj->asStringRaw());
	}

	Object envir = info->get("compilerEnviron");
	if (envir->type() != Script::Type::Undefined)
		compilerEnviron = envir->asStringRaw();

	// Figure out what compiler we're using
	Object comps = info->get("compilers");
	Script::CoerceOrThrow("languageInfo.compilers", comps, Type::Map);
	auto tryCompiler = [&](const string& binary) -> bool {
		string compilerBin = FSUtil::which(binary);
		if (FSUtil::exists(compilerBin)) {
			// Which compiler is this?
			for (Script::ObjectMap::const_iterator it =
				 comps->map->begin(); it != comps->map->end(); it++) {
				Object comp = it->second;
				Script::CoerceOrThrow("languageInfo.compiler", comp, Type::Map);
				Object detect = comp->get("detect");
				Script::CoerceOrThrow("languageInfo.compiler.detect", detect, Type::Map);
				OSUtil::ExecResult res =
					OSUtil::exec(binary, detect->get("arguments")->asStringRaw());
				if (res.exitcode != 0)
					continue; // did not exit with 0 -- something wrong
				Object contains = detect->get("contains");
				Script::CoerceOrThrow("languageInfo.compiler.detect.contains", contains, Type::List);
				bool OK = true;
				for (const Object obj : *contains->list) {
					if (OK && res.output.find(obj->asStringRaw()) == string::npos)
						OK = false;
				}
				if (OK) {
					compilerName = it->first;
					compilerBinary = compilerBin;
					compilerDefaultFlags = comp->get("defaultFlags")->asStringRaw();
					compilerDependenciesFlag = comp->get("dependencies")->asStringRaw();
					compilerDependencyPrefix = (comp->type() != Type::Undefined) ?
						comp->get("dependenciesPrefix")->asStringRaw() : "";
					compilerCompileFlag = comp->get("compile")->asStringRaw();
					compilerOutputFlag = comp->get("output")->asStringRaw();
					compilerOutputExtension = comp->get("outputExtension")->asStringRaw();
					compilerLinkBinaryFlag = comp->get("linkBinary")->asStringRaw();
					compilerDefinition = comp->get("definition")->asStringRaw();
					compilerInclude = comp->get("include")->asStringRaw();

					string depformat = comp->get("dependenciesFormat")->asStringRaw();
					if (depformat == "Makefile")
						compilerDependencyFormat = Generator::MakeFormat;
					else if (depformat == "Stdout")
						compilerDependencyFormat = Generator::StdoutFormat;

					// Update superglobals
					sStack->addSuperglobal(compilerName, Script::BooleanObject(true));
					sStack->get_ptr({"$Compilers"})->map->set(name, Script::StringObject(compilerName));
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
		string env = OSUtil::getEnv(compilerEnviron);
		if (!env.empty())
			tryCompiler(env);
	}
	if (compilerName.empty()) {
		// Preferred & environ didn't work, try everything in succession instead
		for (Script::ObjectMap::const_iterator it =
			 comps->map->begin(); it != comps->map->end(); it++) {
			Object comp = it->second;
			Script::CoerceOrThrow("languageInfo.compiler", comp, Type::Map);
			if (tryCompiler(comp->get("binary")->asStringRaw()))
				break;
		}
	}
	if (compilerName.empty()) {
		PrintUtil::checkFinished("none found", 0);
		throw Script::Exception(Script::Exception::UserError,
			string("cannot find a compiler for " + langName));
	} else
		PrintUtil::checkFinished(compilerName + " ('" + compilerBinary + "')", 2);

	// Grab standards modes
	Object stdsModes = info->map->get_ptr("standardsModes");
	Script::CoerceOrThrow("languageInfo.standardsModes", stdsModes, Type::Map);
	for (Script::ObjectMap::const_iterator it =
		 stdsModes->map->begin(); it != stdsModes->map->end(); it++) {
		StandardsMode mode;
		mode.status = 0;
		Object obj = it->second;
		Script::CoerceOrThrow("languageInfo.standardsModes[i]", obj, Type::Map);
		mode.test = obj->get("test")->asStringRaw();
		Object forComp = obj->map->get_ptr(compilerName);
		Script::CoerceOrThrow("languageInfo.standardsModes[i][comp]", forComp, Type::Map);
		mode.normalFlag = forComp->map->get("normal")->asStringRaw();
		mode.strictFlag = forComp->map->get("strict")->asStringRaw();
		standardsModes.insert({it->first, mode});
	}

	// Check that the compiler works
	PrintUtil::checking("if the " + langName + " compiler works");
	OSUtil::ExecResult res =
		checkIfCompiles("test" + langName, info->get("test")->asStringRaw());
	if (res.exitcode == 0) {
		PrintUtil::checkFinished("yes", 2);
	} else {
		PrintUtil::checkFinished("no", 0);
		throw Script::Exception(Script::Exception::UserError,
			string("complier for " + langName + " is broken: '" + res.output + "'"));
	}
}

bool LanguageInfo::checkStandardsMode(std::string standardsMode)
{
	StandardsMode mode = standardsModes[standardsMode];
	if (mode.status > 0) // If the mode was already checked, it'll have a nonzero status
		return true;
	if (mode.status < 0)
		return false;

	PrintUtil::checking("if the standards mode '" + name + standardsMode + "' works");
	OSUtil::ExecResult res =
		checkIfCompiles("test" + name + standardsMode, mode.test, mode.normalFlag);
	if (res.exitcode == 0) {
		PrintUtil::checkFinished("yes", 2);
		standardsModes[standardsMode].status = 1;
		return true;
	} else {
		PrintUtil::checkFinished("no", 0);
		standardsModes[standardsMode].status = -1;
		return false;
	}
}

void LanguageInfo::generate(Generator* gen)
{
	if (fGenerated)
		return;

	string rule = compilerCompileFlag + "%INPUTFILE% " +
		compilerOutputFlag + "%OUTPUTFILE% %TARGETFLAGS%";
	if (compilerDependencyFormat == Generator::MakeFormat)
		rule = compilerDependenciesFlag + "%OUTPUTFILE%.d " + rule;
	else if (compilerDependencyFormat == Generator::StdoutFormat)
		rule = compilerDependenciesFlag + " " + rule;

	gen->addRegularRule("lang" + genName, name, sourceExtensions,
		compilerBinary, compilerOutputExtension, compilerDependencyFormat,
		compilerDependencyPrefix, rule);
	gen->addLinkRule("link" + genName, "Link" + name, compilerBinary,
		"%INPUTFILE% " + compilerLinkBinaryFlag + "%OUTPUTFILE% %TARGETFLAGS%");

	fGenerated = true;
}

OSUtil::ExecResult LanguageInfo::checkIfCompiles(const string& testName,
	const string& testContents, const string& extraFlags)
{
	string testFileBase = "PhoenixTemp/" + testName;
	FSUtil::setContents(testFileBase + sourceExtensions[0], testContents);
	OSUtil::ExecResult res = OSUtil::exec(compilerBinary,
		extraFlags + " " + testFileBase + sourceExtensions[0] + " " +
		compilerLinkBinaryFlag + testFileBase + APPLICATION_FILE_EXT);
	FSUtil::deleteFile(testFileBase + sourceExtensions[0]);
	bool outFileExisted = FSUtil::exists(testFileBase + APPLICATION_FILE_EXT);
	FSUtil::deleteFile(testFileBase + APPLICATION_FILE_EXT);
#ifdef _WIN32
	if (compilerName == "MSVC") // MSVC leaves behind the .obj file, so delete it.
		FSUtil::deleteFile(testName + ".obj");
#endif
	if (res.exitcode == 0 && !outFileExisted)
		res.exitcode = 1;
	return res;
}

LanguageInfo* LanguageInfo::getLanguageInfo(string langName)
{
	if (sData.count(langName) != 0)
		return sData[langName];

	// FIXME: this is bootstrap-only Phoenix, load hardcoded location
	Object info = Script::Run(sStack,
		FSUtil::combinePaths({FSUtil::parentDirectory(__FILE__),
			"../../data/languages/" + langName + ".phnx"}));
	LanguageInfo* langInfo = new LanguageInfo(langName, info);
	sData.insert({langName, langInfo});
	return langInfo;
}
