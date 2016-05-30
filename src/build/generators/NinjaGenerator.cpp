/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "NinjaGenerator.h"

#include "Phoenix.h"
#include "util/FSUtil.h"
#include "util/StringUtil.h"

using std::string;
using std::vector;

NinjaGenerator::NinjaGenerator()
{
}

string NinjaGenerator::name()
{
	return "Ninja";
}

string NinjaGenerator::escapeString(const string& str)
{
	string ret = str;
	StringUtil::replaceAll(ret, ":", "$:");
	StringUtil::replaceAll(ret, " ", "$ ");
	return ret;
}

void NinjaGenerator::setBuildScriptFiles(string program, const vector<string> files)
{
	fRulesLines.push_back("rule RERUN_PHOENIX\n"
		"  command = " + program + "\n"
		"  description = Re-running Phoenix...\n"
		"  generator = 1"
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

void NinjaGenerator::addObjectRule(const string& ruleName, const string& descName,
	const vector<string>& forExts, const string& program, const string& outFileExt,
	const string& rule)
{
	string realRule = rule;
	StringUtil::replaceAll(realRule, "%INPUTFILE%", "$in");
	StringUtil::replaceAll(realRule, "%OUTPUTFILE%", "$out");
	StringUtil::replaceAll(realRule, "%TARGETFLAGS%", "$targetflags");
	fRulesLines.push_back("rule " + ruleName + "\n"
		"  command = " + program + " " + realRule + "\n"
		"  description = " + descName + " $out\n"
		//"  depfile = $out.d\n"
		//"  deps = gcc\n"
		);

	RuleForExt itm;
	itm.outFileExt = outFileExt;
	itm.ruleName = ruleName;
	for (string ext : forExts)
		fRulesForExts.insert({ext, itm});
}

void NinjaGenerator::setProgramLinkRule(const string& rule)
{
	string realRule = rule;
	StringUtil::replaceAll(realRule, "%INPUTFILE%", "$in");
	StringUtil::replaceAll(realRule, "%OUTPUTFILE%", "$out");
	StringUtil::replaceAll(realRule, "%TARGETFLAGS%", "$targetflags");
	fRulesLines.push_back("rule link\n"
		"  command = " + realRule + "\n"
		"  description = LINK $out");
}

void NinjaGenerator::addTarget(const string& outputBinaryName,
	const vector<string>& inputFiles, const string& targetFlags)
{
	vector<string> outfiles;
	string targetflagsvar = "tf_" + outputBinaryName;
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
	fBuildLines.push_back("build build-" + outputBinaryName + "/" + outputBinaryName + ": " +
		"link " + StringUtil::join(outfiles, " ") + "\n");
}

vector<string> NinjaGenerator::outputFiles()
{
	return {"build.ninja"};
}
void NinjaGenerator::write()
{
	FSUtil::putContents("build.ninja",
		"# This file was automatically generated by Phoenix " PHOENIX_VERSION "\n"
		"# ALL CHANGES WILL BE LOST ON NEXT REGENERATION!\n\n"
		"targetflags = \n\n" +
		StringUtil::join(fRulesLines, "\n") + "\n" +
		StringUtil::join(fBuildLines, "\n") + "\n");
}
