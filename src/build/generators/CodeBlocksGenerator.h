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

	virtual void setBuildScriptFiles(const std::string&,
		const std::vector<std::string> files) override;

	virtual void addRegularRule(const std::string&,
		const std::string&, const std::vector<std::string>&,
		const std::string&, const std::string&,
		const std::string&) override {}
	virtual void addLinkRule(const std::string&,
		const std::string&, const std::string&,
		const std::string&) override {}

	virtual void addTarget(const std::string&,
		const std::string& outputBinaryName,
	    const std::vector<std::string>& inputFiles, const std::string&,
	    Target* target) override;

	virtual std::vector<std::string> outputFiles() override;

	virtual void write() override;

private:
	std::string fCompiler;
	std::vector<std::string> fTargets;
	typedef std::map<std::string, std::vector<std::string>> _filesmap;
	_filesmap fFilesAndTargets;
};
