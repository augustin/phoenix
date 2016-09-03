/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Generators.h"

#include "generators/CodeBlocksGenerator.h"
#include "generators/NinjaGenerator.h"

#include "util/PrintUtil.h"

using std::string;
using std::vector;

Generator::~Generator()
{
}
string Generator::command(const string& target)
{
	return "";
}

class MultiGenerator : public Generator
{
public:
	MultiGenerator(Generator* primary) : fPrimary(primary) {}
	virtual ~MultiGenerator() {
		delete fPrimary;
		for (Generator* gen : fSecondaries)
			delete gen;
	}
	void add(Generator* gen) { fSecondaries.push_back(gen); }

	virtual bool check() override {
		bool ok = fPrimary->check();
		for (Generator* gen : fSecondaries)
			ok = ok && gen->check();
		return ok;
	}
	virtual void setProjectName(const string& name) {
		fPrimary->setProjectName(name);
		for (Generator* gen : fSecondaries)
			gen->setProjectName(name);
	}
	virtual void setBuildScriptFiles(const string& program,
		const vector<string> files) override {
		fPrimary->setBuildScriptFiles(program, files);
		for (Generator* gen : fSecondaries)
			gen->setBuildScriptFiles(program, files);
	}
	virtual void addRegularRule(const string& ruleName,
		const string& descName, const vector<string>& forExts,
		const string& program, const string& outFileExt,
		DependencyFormat depFormat, const std::string& depPrefix,
		const string& rule) override {
		fPrimary->addRegularRule(ruleName, descName, forExts, program,
			outFileExt, depFormat, depPrefix, rule);
		for (Generator* gen : fSecondaries)
			gen->addRegularRule(ruleName, descName, forExts, program,
				outFileExt, depFormat, depPrefix, rule);
	}
	virtual void addLinkRule(const string& ruleName,
		const string& descName, const string& program,
		const string& rule) override {
		fPrimary->addLinkRule(ruleName, descName, program, rule);
		for (Generator* gen : fSecondaries)
			gen->addLinkRule(ruleName, descName, program, rule);
	}
	virtual void addTarget(const string& linkRule,
		const string& outputBinaryName,
		const vector<string>& inputFiles,
		const string& targetFlags, const Target* target) override {
		fPrimary->addTarget(linkRule, outputBinaryName, inputFiles, targetFlags, target);
		for (Generator* gen : fSecondaries)
			gen->addTarget(linkRule, outputBinaryName, inputFiles, targetFlags, target);
	}
	virtual vector<string> outputFiles() override {
		vector<string> ret = fPrimary->outputFiles();
		for (Generator* gen : fSecondaries) {
			vector<string> vec = gen->outputFiles();
			ret.insert(ret.end(), vec.begin(), vec.end());
		}
		return ret;
	}
	virtual string command(const std::string& target) override {
		return fPrimary->command(target);
	}
	virtual void write() override {
		fPrimary->write();
		for (Generator* gen : fSecondaries)
			gen->write();
	}
private:
	Generator* fPrimary;
	vector<Generator*> fSecondaries;
};


// Static member variables
Generator* Generators::primary = nullptr;
Generator* Generators::actual = nullptr;

string Generators::defaultName()
{
	return "Ninja";
}

vector<string> Generators::list()
{
	return {"Ninja"};
}
vector<string> Generators::listSecondary()
{
	return {"CodeBlocks"};
}

Generator* Generators::create(string name, vector<string> secondary)
{
	Generator* ret = nullptr;
	if (name == "Ninja")
		ret = new NinjaGenerator;
	else {
		PrintUtil::error("there is no primary generator with name '" + name + "'");
		return nullptr;
	}
	primary = ret;
	actual = ret;

	if (secondary.empty())
		return ret;

	ret = new MultiGenerator(ret);
	for (string gen : secondary) {
		if (gen == "CodeBlocks")
			static_cast<MultiGenerator*>(ret)->add(new CodeBlocksGenerator);
		else
			PrintUtil::warning("there is no secondary generator with name '" + gen + "'");
	}
	actual = ret;
	return ret;
}
