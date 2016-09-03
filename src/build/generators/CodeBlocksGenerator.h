/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "build/Generators.h"

class CodeBlocksGenerator : public Generator
{
public:
	CodeBlocksGenerator();
	virtual ~CodeBlocksGenerator();

	virtual bool check() override;

	virtual void setProjectName(const std::string& name) override;

	virtual void setBuildScriptFiles(const std::string&,
		const std::vector<std::string> files) override;

	virtual void addRegularRule(const std::string&,
		const std::string&, const std::vector<std::string>&,
		const std::string&, const std::string&,
		DependencyFormat, const std::string&,
		const std::string&) override {}
	virtual void addLinkRule(const std::string&,
		const std::string&, const std::string&,
		const std::string&) override {}

	virtual void addTarget(const std::string&,
	    const std::string& outputBinaryName,
	    const std::vector<std::string>& inputFiles, const std::string&,
	    const Target* target) override;

	virtual std::vector<std::string> outputFiles() override;

	virtual void write() override;

private:
	std::string fName;
	std::string fCompiler;

	typedef struct {
		std::string name;
		std::vector<std::string> includeDirs;
		std::string compiler;
	} CodeBlocksTarget;
	std::vector<CodeBlocksTarget> fTargets;

	typedef std::map<std::string, std::vector<std::string>> _multimap;
	_multimap fFilesAndTargets; /* map: file, target(s) */
};
