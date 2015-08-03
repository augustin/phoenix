/*
 * (C) 2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <iostream>
#include <vector>

#include "Phoenix.h"
#include "generators/Generators.h"
#include "util/FSUtil.h"

using std::vector;
using std::string;
using std::cerr;

void printUsage(string program)
{
	cerr << "Phoenix, version " PHOENIX_VERSION << ". ";
	cerr << "(C) 2015 Augustin Cavalier." << std::endl << std::endl;
	cerr << "Usage: " << std::endl;
	cerr << "\t" << program << " [options] <path/to/source>" << std::endl;
	cerr << "\t" << program << " [options] <path/to/build/directory>" << std::endl;
}

int main(int argc, char* argv[])
{
	vector<string> arguments(argv, argv + argc);
	if (arguments.size() < 2) {
		printUsage(arguments[0]);
		cerr << std::endl;
		cerr << "Run '" << arguments[0] << " --help' for full usage information.";
		cerr << std::endl;
		return 1;
	}

	string directory = ".";
	for (int i = 1; i < argc; i++) {
		string arg = arguments[i];
		if (arg.find("--help") == 0) {
			printUsage(arguments[0]);
			cerr << "Options: " << std::endl;
			cerr << "\t-G<generator>\t\tBuild system generator to use (default: '" <<
				Generators::defaultName() << "')." << std::endl;
			// cerr << "\t-X<target>\tCross-compile to <target>." << std::endl; // TODO
			cerr << "\t-C:<lang>:<compiler>\tCompiler for <lang> to use (default depends on target)." <<
				std::endl;
			return 1;
		} else {
			std::cout << FSUtil::which(arg);
		}
	}

	return 0;
}
