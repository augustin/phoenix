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

	virtual void addObjectRule(const std::string& ruleName,
		const std::string& descName, const std::vector<std::string>& forExts,
		const std::string& program, const std::string& outFileExt,
		const std::string& rule);
	virtual void setProgramLinkRule(const std::string& rule);

	virtual void addTarget(const std::string& outputBinaryName,
		const std::vector<std::string>& inputFiles,
		const std::string& targetFlags);

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
