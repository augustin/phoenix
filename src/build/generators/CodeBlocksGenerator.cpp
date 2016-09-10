/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "CodeBlocksGenerator.h"

#include <algorithm>

#include "Phoenix.h"

#include "build/Target.h"
#include "build/LanguageInfo.h"
#include "util/FSUtil.h"
#include "util/XmlUtil.h"

using std::string;
using std::vector;

CodeBlocksGenerator::CodeBlocksGenerator()
	:
	fName("Project")
{
}
CodeBlocksGenerator::~CodeBlocksGenerator()
{
}

bool CodeBlocksGenerator::check()
{
	// CodeBlocks may or may not be installed, or we may be generating
	// files for something else altogether (e.g. Qt Creator), so just
	// return true reguardeless.
	return true;
}

void CodeBlocksGenerator::setProjectName(const string& name)
{
	fName = name;
}

void CodeBlocksGenerator::setBuildScriptFiles(const string&, const vector<string> files)
{
	for (string file : files)
		fFilesAndTargets.insert({file, {}});
}

void CodeBlocksGenerator::addTarget(const string&, const string& outputBinaryName,
	const vector<string>& inputFiles, const string&, const Target* target)
{
	// TODO: get rid of hardcoded languages[0]?
	std::string compiler = LanguageInfo::getLanguageInfo(target->languages[0])->compilerName;
	std::transform(compiler.begin(), compiler.end(), compiler.begin(), ::tolower); // Do we need this?
	if (fCompiler.empty())
		fCompiler = compiler;

	CodeBlocksTarget t;
	t.name = outputBinaryName;
	t.includeDirs = target->includeDirs;
	t.compiler = compiler;
	fTargets.push_back(t);

	for (string file : inputFiles) {
		_multimap::iterator it = fFilesAndTargets.find(file);
		if (it != fFilesAndTargets.end())
			it->second.push_back(outputBinaryName);
		else
			fFilesAndTargets.insert({file, {outputBinaryName}});
	}
	for (string file : target->extraFiles) {
		_multimap::iterator it = fFilesAndTargets.find(file);
		if (it != fFilesAndTargets.end())
			it->second.push_back(outputBinaryName);
		else
			fFilesAndTargets.insert({file, {outputBinaryName}});
	}
}

vector<string> CodeBlocksGenerator::outputFiles()
{
	return {fName + ".cbp"};
}
void CodeBlocksGenerator::write()
{
	XmlGenerator gen("CodeBlocks_project_file");
	gen.beginTag("FileVersion", {{"major", "1"}, {"minor", "6"}}, true);
	gen.beginTag("Project");
	gen.beginTag("Option", {{"title", fName}}, true);
	gen.beginTag("Option", {{"makefile_is_custom", "1"}}, true);
	gen.beginTag("Option", {{"compiler", fCompiler}}, true);

	gen.beginTag("Build");
		gen.beginTag("Target", {{"title", "all"}});
			gen.beginTag("Option", {{"type", "4"}}, true);
			gen.beginTag("MakeCommands");
				gen.beginTag("Build", {{"command",
					Generators::primary->command("all")}}, true);
				gen.beginTag("CompileFile", {{"command",
					Generators::primary->command("\"$file\"")}}, true);
				gen.beginTag("Clean", {{"command",
					Generators::primary->command("clean")}}, true);
				//gen.beginTag("DistClean", {{"command", }}, true); // TODO?
			gen.endTag("MakeCommands");
		gen.endTag("Target");

		for (CodeBlocksTarget target : fTargets) {
			gen.beginTag("Target", {{"title", target.name}});
				// TODO: runtime_output_dir
				gen.beginTag("Option", {{"output", FSUtil::absolutePath(target.name)},
					{"prefix_auto", "0"}, {"extension_auto", "0"}}, true);
				gen.beginTag("Option", {{"compiler", target.compiler}}, true);

				gen.beginTag("MakeCommands");
					gen.beginTag("Build", {{"command",
						Generators::primary->command(target.name)}}, true);
					gen.beginTag("CompileFile", {{"command",
						Generators::primary->command("\"$file\"")}}, true);
					gen.beginTag("Clean", {{"command",
						Generators::primary->command("clean")}}, true);
					//gen.beginTag("DistClean", {{"command", }}, true); // TODO?
				gen.endTag("MakeCommands");

				gen.beginTag("Compiler");
					for (string dir : target.includeDirs)
						gen.beginTag("Add", {{"directory", dir}}, true);
				gen.endTag("Compiler");
			gen.endTag("Target");
		}
	gen.endTag("Build");

	for (_multimap::const_iterator it = fFilesAndTargets.begin(); it != fFilesAndTargets.end(); it++) {
		gen.beginTag("Unit", {{"filename", it->first}}, it->second.empty());
		for (string target : it->second)
			gen.beginTag("Option", {{"target", target}}, true);
		if (!it->second.empty())
			gen.endTag("Unit");
	}
	FSUtil::setContents(fName + ".cbp", gen.finish());
}
