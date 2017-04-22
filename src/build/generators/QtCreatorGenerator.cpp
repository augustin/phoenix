/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QtCreatorGenerator.h"

#include <algorithm>

#include "Phoenix.h"

#include "build/Target.h"
#include "build/LanguageInfo.h"
#include "util/FSUtil.h"

using std::string;
using std::vector;

QtCreatorGenerator::QtCreatorGenerator()
	:
	fName("Project")
{
}
QtCreatorGenerator::~QtCreatorGenerator()
{
}

bool QtCreatorGenerator::check()
{
	// Nothing to check here.
	return true;
}

void QtCreatorGenerator::setProjectName(const string& name)
{
	fName = name;
}

void QtCreatorGenerator::setBuildScriptFiles(const string&, const vector<string> files)
{
	for (const string& file : files)
		fFiles.push_back(file);
}

void QtCreatorGenerator::addTarget(const string&, const string& outputBinaryName,
	const vector<string>& inputFiles, const string&, const Target* target)
{
	for (const string& file : inputFiles) {
		if (std::find(fFiles.begin(), fFiles.end(), file) == fFiles.end())
			fFiles.push_back(file);
	}
	for (const string& file : target->extraFiles) {
		if (std::find(fFiles.begin(), fFiles.end(), file) == fFiles.end())
			fFiles.push_back(file);
	}
	for (const string& dir : target->includeDirs) {
		if (std::find(fIncludeDirs.begin(), fIncludeDirs.end(), dir) == fIncludeDirs.end())
			fIncludeDirs.push_back(dir);
	}
}

vector<string> QtCreatorGenerator::outputFiles()
{
	return {
		fName + ".creator",
		fName + ".config",	// TODO: we don't put anything in here yet.
		fName + ".files",
		fName + ".includes",
	};
}
void QtCreatorGenerator::write()
{
	string dotCreator = "[General]\n";
	FSUtil::setContents(fName + ".creator", dotCreator);

	FSUtil::setContents(fName + ".config", "");

	string dotFiles;
	for (const string& file : fFiles)
		dotFiles.append(file + "\n");
	FSUtil::setContents(fName + ".files", dotFiles);

	string dotIncludes;
	for (const string& dir : fIncludeDirs)
		dotIncludes.append(dir + "\n");
	FSUtil::setContents(fName + ".includes", dotIncludes);
}
