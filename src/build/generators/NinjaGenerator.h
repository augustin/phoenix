/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "build/Generators.h"

class NinjaGenerator : public Generator
{
public:
	NinjaGenerator();

	virtual std::string name() override;

	virtual void addObjectRule(std::string ruleName, std::string descName,
		std::vector<std::string> forExts, std::string program,
		std::string outFileExt, std::string rule) override;
	virtual void setProgramLinkRule(std::string rule) override;

	virtual void addTarget(std::string outputBinaryName,
		std::vector<std::string> inputFiles, std::string targetFlags) override;

	virtual void write() override;

private:
	typedef struct {
		std::string ruleName;
		std::string outFileExt;
	} RuleForExt;
	std::map<std::string, RuleForExt> fRulesForExts;

	std::vector<std::string> fRulesLines;
	std::vector<std::string> fBuildLines;
};
