/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "build/Generators.h"

class QtCreatorGenerator : public Generator
{
public:
	QtCreatorGenerator();
	virtual ~QtCreatorGenerator();

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

	std::vector<std::string> fFiles;
	std::vector<std::string> fIncludeDirs;
};
