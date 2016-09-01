/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "CodeBlocksGenerator.h"

#include "Phoenix.h"

#include "util/FSUtil.h"
#include "util/XmlUtil.h"

using std::string;
using std::vector;

CodeBlocksGenerator::CodeBlocksGenerator()
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

void CodeBlocksGenerator::setBuildScriptFiles(const string&, const vector<string> files)
{
	for (string file : files)
		fFilesAndTargets.insert({file, {}});
}

void CodeBlocksGenerator::addTarget(const string&, const string& outputBinaryName,
	const vector<string>& inputFiles, const string&)
{
	fTargets.push_back(outputBinaryName);
	for (string file : inputFiles) {
		_filesmap::iterator it = fFilesAndTargets.find(file);
		if (it != fFilesAndTargets.end())
			it->second.push_back(outputBinaryName);
		else
			fFilesAndTargets.insert({file, {outputBinaryName}});
	}
}

vector<string> CodeBlocksGenerator::outputFiles()
{
	return {"Project.cbp"};
}
void CodeBlocksGenerator::write()
{
	XmlGenerator gen("CodeBlocks_project_file");
	gen.beginTag("FileVersion", {{"major", "1"}, {"minor", "6"}}, true);
	gen.beginTag("Project");
	gen.beginTag("Option", {{"title", "Project"}}, true);
	gen.beginTag("Option", {{"makefile_is_custom", "1"}}, true);
	// gen.beginTag("Option", {{"compiler", "gcc"}}, true); // TODO?

	gen.beginTag("Build");
		gen.beginTag("Target", {{"title", "all"}});
			//gen.beginTag("Option", {{"working_dir", "..."}}, true); // TODO?
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

		for (string target : fTargets) {
			gen.beginTag("Target", {{"title", target}});
				gen.beginTag("Option", {{"output", target},
					{"prefix_auto", "0"}, {"extension_auto", "0"}}, true);

				gen.beginTag("MakeCommands");
					gen.beginTag("Build", {{"command",
						Generators::primary->command(target)}}, true);
					gen.beginTag("CompileFile", {{"command",
						Generators::primary->command("\"$file\"")}}, true);
					gen.beginTag("Clean", {{"command",
						Generators::primary->command("clean")}}, true);
					//gen.beginTag("DistClean", {{"command", }}, true); // TODO?
				gen.endTag("MakeCommands");
			gen.endTag("Target");
		}
	gen.endTag("Build");

	for (_filesmap::const_iterator it = fFilesAndTargets.begin(); it != fFilesAndTargets.end(); it++) {
		gen.beginTag("Unit", {{"filename", it->first}}, it->second.empty());
		for (string target : it->second)
			gen.beginTag("Option", {{"target", target}}, true);
		if (!it->second.empty())
			gen.endTag("Unit");
	}
	FSUtil::putContents("Project.cbp", gen.finish());
}
