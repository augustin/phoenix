/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "NinjaGenerator.h"

#include "Phoenix.h"

#include "util/FSUtil.h"
#include "util/StringUtil.h"
#include "util/PrintUtil.h"
#include "util/OSUtil.h"

using std::string;
using std::vector;

NinjaGenerator::NinjaGenerator()
	:
	fFeaturePoolConsole(false)
{
}
NinjaGenerator::~NinjaGenerator()
{
}

bool NinjaGenerator::check()
{
	PrintUtil::checking("if Ninja is installed and working");
	fNinjaExecutable = FSUtil::which("ninja");
	if (fNinjaExecutable.empty()) {
		PrintUtil::checkFinished("not installed", 0);
		PrintUtil::error("Ninja is either not installed or not in PATH.");
		return false;
	}
	OSUtil::ExecResult e = OSUtil::exec(fNinjaExecutable, "--version");
	if (e.exitcode != 0) {
		PrintUtil::checkFinished("failed to get version", 0);
		PrintUtil::error("'ninja --version' did not exit with 0, something is very wrong.");
		return false;
	}
	string version = StringUtil::trim(e.output);
	if (version < "1.3.0") {
		PrintUtil::checkFinished("too old", 0);
		PrintUtil::error("the installed Ninja is v" + version + ", Phoenix requires v1.3.0 or better");
		return false;
	}
	if (version >= "1.5.0") {
		PrintUtil::checkFinished("yes", 2);
		fRequiredVersion = "1.5";
		fFeaturePoolConsole = true;
	} else {
		PrintUtil::checkFinished("old, but acceptable", 1);
		fRequiredVersion = "1.3";
	}
	return true;
}

string NinjaGenerator::escapeString(const string& str)
{
	string ret = str;
	StringUtil::replaceAll(ret, ":", "$:");
	StringUtil::replaceAll(ret, " ", "$ ");
	return ret;
}

void NinjaGenerator::setBuildScriptFiles(const string& program, const vector<string> files)
{
	fRulesLines.push_back("rule RERUN_PHOENIX\n"
		"  command = " + program + "\n"
		"  description = Re-running Phoenix...\n"
		"  generator = 1\n" +
		(fFeaturePoolConsole ? "  pool = console\n" : "")
		);
	string phony = "build"; // So it doesn't error out if a file is missing
	string build = "build build.ninja: RERUN_PHOENIX |";
	for (string file : files) {
		string add = " " + escapeString(file);
		build += add;
		phony += add;
	}
	phony += ": phony\n";
	fBuildLines.push_back(build);
	fBuildLines.push_back(phony);
}

void NinjaGenerator::addRegularRule(const string& ruleName, const string& descName,
	const vector<string>& forExts, const string& program, const string& outFileExt,
	DependencyFormat depFormat,	const std::string& depPrefix, const string& rule)
{
	if (depFormat == StdoutFormat && !depPrefix.empty() && fDepsPrefix.empty())
		fDepsPrefix = depPrefix;

	string realRule = rule;
	StringUtil::replaceAll(realRule, "%INPUTFILE%", "$in");
	StringUtil::replaceAll(realRule, "%OUTPUTFILE%", "$out");
	StringUtil::replaceAll(realRule, "%TARGETFLAGS%", "$targetflags");

	string ruleLine = "rule " + ruleName + "\n"
	   "  command = " + program + " " + realRule + "\n"
	   "  description = " + descName + " $out\n";
	if (depFormat == MakeFormat) {
		ruleLine += "  depfile = $out.d\n"
			"  deps = gcc\n";
	} else if (depFormat == StdoutFormat) {
		ruleLine += "  deps = msvc\n";
	}
	fRulesLines.push_back(ruleLine);

	RuleForExt itm;
	itm.outFileExt = outFileExt;
	itm.ruleName = ruleName;
	for (string ext : forExts)
		fRulesForExts.insert({ext, itm});
}

void NinjaGenerator::addLinkRule(const string& ruleName,
	const string& descName, const string& program, const string& rule)
{
	string realRule = rule;
	StringUtil::replaceAll(realRule, "%INPUTFILE%", "$in");
	StringUtil::replaceAll(realRule, "%OUTPUTFILE%", "$out");
	StringUtil::replaceAll(realRule, "%TARGETFLAGS%", "$targetflags");
	fRulesLines.push_back("rule " + ruleName + "\n"
		"  command = " + program + " " + realRule + "\n"
		"  description = " + descName + " $out");
}

void NinjaGenerator::addTarget(const string& linkRule, const string& outputBinaryName,
	const vector<string>& inputFiles, const string& targetFlags, const Target*)
{
	vector<string> outfiles;
	string targetflagsvar = "tf_" + StringUtil::split(outputBinaryName, ".")[0];
	if (!targetFlags.empty())
		fBuildLines.push_back(targetflagsvar + " = " + targetFlags);
	for (string file : inputFiles) {
		vector<string> splitByDot = StringUtil::split(file, ".");
		vector<string> splitBySlash = StringUtil::split(file, "/");
		string ext = "." + splitByDot[splitByDot.size() - 1];
		RuleForExt rule = fRulesForExts[ext];
		string outFile = "build-" + outputBinaryName + "/" +
			splitBySlash[splitBySlash.size() - 1] + rule.outFileExt;
		outfiles.push_back(outFile);

		string line = "build " + escapeString(outFile) + ": " +
			rule.ruleName + " " + escapeString(file);
		if (targetflagsvar.length())
			line += "\n  targetflags = $" + targetflagsvar;
		fBuildLines.push_back(line);
	}
	std::string targetFile = /* TODO: runtimeOutputDirectory */ outputBinaryName;
	fBuildLines.push_back("build " + targetFile + ": " + linkRule +
		" " + StringUtil::join(outfiles, " ") + "\n");
	fTargets.push_back(targetFile);
}

vector<string> NinjaGenerator::outputFiles()
{
	return {"build.ninja"};
}
string NinjaGenerator::command(const string& target)
{
	return fNinjaExecutable + (target.empty() ? "" : " " + target);
}

void NinjaGenerator::write()
{
	FSUtil::setContents("build.ninja",
		"# This file was automatically generated by Phoenix " PHOENIX_VERSION "\n"
		"# ALL CHANGES WILL BE LOST ON NEXT REGENERATION!\n"
		"ninja_required_version = " + fRequiredVersion + "\n\n" +

		// Setup stuff
		(fDepsPrefix.empty() ? "" : "msvc_deps_prefix = " + fDepsPrefix + "\n") +

		// Default targets/commands
		"rule CLEAN\n"
		"  command = ninja -t clean\n"
		"  description = Cleaning all built files...\n"
		"build clean: CLEAN\n\n"

		// Default variables
		"targetflags = \n\n" +

		// Actual build stuff
		StringUtil::join(fRulesLines, "\n") + "\n" +
		StringUtil::join(fBuildLines, "\n") + "\n" +

		// "all" target & target defaults
		"build all: phony " + StringUtil::join(fTargets, " ") + "\n" +
		"default all\n");
}
