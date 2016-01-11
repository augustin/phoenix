/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <iostream>
#include <vector>

#include "Phoenix.h"

#ifdef _WIN32
#error It's Win32!
#endif

#include "build/Generators.h"
#include "build/LanguageInfo.h"
#include "build/Target.h"

#include "language/GlobalLanguageObject.h"
#include "language/Parser.h"
#include "language/Stack.h"

#include "util/FSUtil.h"
#include "util/StringUtil.h"
#include "util/PrintUtil.h"

using std::vector;
using std::string;
using std::cerr;

void printUsage(string program)
{
	cerr << "Phoenix, version " PHOENIX_VERSION << ". ";
	cerr << "(C) 2015-2016 Augustin Cavalier." << std::endl << std::endl;
	cerr << "Usage: " << std::endl;
	cerr << "\t" << program << " [options] <path/to/source> [<path/to/build/directory>]" << std::endl;
	cerr << "\t" << program << " [options] <path/to/build/directory = .>" << std::endl;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "C"); // Force C locale

	vector<string> arguments(argv, argv + argc);
	if (arguments.size() < 2) {
		printUsage(arguments[0]);
		cerr << std::endl;
		cerr << "Run '" << arguments[0] << " --help' for full usage information.";
		cerr << std::endl;
		return 1;
	}

	string buildDirectory = ".", sourceDirectory = "";
	for (int i = 1; i < argc; i++) {
		string arg = arguments[i];
		if (arg == "--help") {
			printUsage(arguments[0]);
			cerr << "Options: " << std::endl;
			cerr << "\t-G<generator>\t\tBuild system generator to use (default: '" <<
				Generators::defaultName() << "')." << std::endl;
			// cerr << "\t-X<target>\tCross-compile to <target>." << std::endl; // TODO
			cerr << "\t-C:<lang>:<compiler>\tPreferred ompiler for <lang> to use." <<
				std::endl;
			return 1;
		} else if (StringUtil::startsWith(arg, "-C:")) {
			vector<string> item = StringUtil::split(arg, ":");
			if (item.size() != 3) {
				cerr << "error: -C passed but did not have the expected -C:<lang>:<compiler> syntax.";
				return 1;
			}
			LanguageInfo::sPreferredCompiler.insert({item[1], item[2]});
		} else if (arg.find(".") != string::npos || arg.find("/") != string::npos ||
				   arg.find("\\") != string::npos) {
			// Looks like a path.
			if (sourceDirectory.length() > 0)
				buildDirectory = arg;
			else
				sourceDirectory = arg;
		} else {
			cerr << "error: unrecognized option '" << arg << "'.";
			return 1;
		}
	}

	if (sourceDirectory.empty() || buildDirectory.empty()) {
		cerr << "No source directory specified!";
		return 1;
	}

	// Create the language stack on the heap, as it can get pretty large.
	Language::Stack* stack = new Language::Stack();
	stack->addSuperglobal("Phoenix", Language::GlobalLanguageObject());
	Target::addGlobalFunction();
	try {
		Language::Run(stack, sourceDirectory);
	} catch (Language::Exception e) {
		e.print();
		return e.fType;
	}

	string generatorName = Generators::defaultName();
	std::cout << "generating build files for " << generatorName << "... ";
	Generator* gen = Generators::create(generatorName);
	for (Target::ExtraData* data : Target::targets)
		Target::generate(data, gen);
	gen->write();
	std::cout << "done" << std::endl;

	return 0;
}
