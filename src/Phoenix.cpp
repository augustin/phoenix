/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Phoenix.h"

#include <clocale>
#include <iostream>
#include <vector>

#include "build/Generators.h"
#include "build/LanguageInfo.h"
#include "build/Target.h"

#include "script/Interpreter.h"
#include "script/Debugger.h"
#include "script/Stack.h"

#include "util/FSUtil.h"
#include "util/StringUtil.h"
#include "util/PrintUtil.h"
#include "util/TermUtil.h"

using std::vector;
using std::string;
using std::cerr;

void printUsage(string program, bool full)
{
	cerr << "Phoenix, version " PHOENIX_VERSION << ". ";
	cerr << "(C) 2015-2017 Augustin Cavalier." << std::endl << std::endl;
	cerr << "Usage: " << std::endl;
	cerr << "\t" << program << " [options] <path/to/source> [<path/to/build/directory = .>]" << std::endl;
	//cerr << "\t" << program << " [options] <path/to/build/directory = .>" << std::endl; // TODO

	if (!full)
		return;
	cerr << "Options: " << std::endl;

	cerr << "\t-G<generator>\t\tBuild system generator to use; possibilities:" << std::endl;
	string defGen = Generators::defaultName();
	vector<string> list = Generators::list();
	for (string genName : list) {
		cerr << "\t\t" << genName;
		if (genName == defGen)
			cerr << " (default)";
		cerr << std::endl;
	}
	cerr << "\t-S<secondary>\t\tSecondary (IDEs, etc.) generator to use; possibilities:" << std::endl;
	list = Generators::listSecondary();
	for (string genName : list)
		cerr << "\t\t" << genName << std::endl;

	// cerr << "\t-X<target>\tCross-compile to <target>." << std::endl; // TODO
	cerr << "\t-C:<lang>:<compiler>\tPreferred compiler for <lang> to use." <<
		std::endl;
	cerr << "\t--debugger\tLaunch into the interactive Phoenix script debugger." <<
		std::endl;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "C"); // Force C locale
	TermUtil::init(); // Set up the terminal, if we're on a TTY

	vector<string> arguments(argv, argv + argc);

	if (arguments.size() < 2) {
		printUsage(arguments[0], false);
		cerr << std::endl;
		cerr << "Run '" << arguments[0] << " --help' for full usage information.";
		cerr << std::endl;
		return 1;
	}

	string buildDirectory = ".", sourceDirectory, generator;
	vector<string> secondaryGenerators;
	bool debugger = false;
	for (vector<string>::size_type i = 1; i < arguments.size(); i++) {
		string arg = arguments[i];
		if (arg == "--help") {
			printUsage(arguments[0], true);
			return 0;
		} else if (arg == "--debugger") {
			debugger = true;
		} else if (StringUtil::startsWith(arg, "-C:")) {
			vector<string> item = StringUtil::split(arg, ":");
			if (item.size() != 3) {
				PrintUtil::error("-C passed but did not have the expected -C:<lang>:<compiler> syntax.");
				return 1;
			}
			LanguageInfo::sPreferredCompiler.insert({item[1], item[2]});
		} else if (StringUtil::startsWith(arg, "-G")) {
			generator = arg.substr(2);
		} else if (StringUtil::startsWith(arg, "-S")) {
			secondaryGenerators.push_back(arg.substr(2));
		} else if (sourceDirectory.empty() || buildDirectory == ".") {
			// Assume it's a path.
			if (!sourceDirectory.empty())
				buildDirectory = arg;
			else
				sourceDirectory = arg;
		} else {
			PrintUtil::error("unrecognized option '" + arg + "'.");
			return 1;
		}
	}

	if (sourceDirectory.empty() || buildDirectory.empty()) {
		PrintUtil::error("no source directory specified");
		return 1;
	}
	if (generator.empty())
		generator = Generators::defaultName();

	// Directory setup
	FSUtil::mkdir("PhoenixTemp");

	Script::Stack* stack = new Script::Stack();
	Target::addGlobalFunction(stack);
	LanguageInfo::sStack = stack;
	stack->addSuperglobal("Compilers", Script::MapObject(new Script::ObjectMap()));

	try {
		Generator* gen = Generators::create(generator, secondaryGenerators);
		if (gen == nullptr) {
			FSUtil::rmdir("PhoenixTemp");
			return 1;
		}
		if (!gen->check()) {
			FSUtil::rmdir("PhoenixTemp");
			return 1;
		}

		if (!debugger) {
			Script::Run(stack, sourceDirectory);
		} else {
			Script::Debugger(stack, sourceDirectory);
		}

		std::cout << "generating build files for " << generator;
		for (string gen : secondaryGenerators)
			std::cout << ", " + gen;
		std::cout << "... ";
		gen->setBuildScriptFiles(StringUtil::join(arguments, " "), stack->inputFiles());
		for (Target* target : Target::targets)
			target->generate(gen);
		gen->write();
		std::cout << "done" << std::endl;
	} catch (Script::Exception e) {
		e.print();
		FSUtil::rmdir("PhoenixTemp");
		return e.fType;
	}

	// Deinitialization
	FSUtil::rmdir("PhoenixTemp");
	delete stack;

	return 0;
}
