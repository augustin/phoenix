/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <clocale>
#include <iostream>

#include "util/FSUtil.h"
#include "util/OSUtil.h"
#include "util/StringUtil.h"
#include "Phoenix.h"

using std::vector;
using std::string;
using std::cerr;

#define NORMALLY_NOTICE \
	cerr << "Normally this part would tell you to run --help for usage, but " \
		"this is actually Phoenix running in Fake CMake mode, so that wouldn't help you." \
		 << std::endl

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "C"); // Force C locale
	vector<string> arguments(argv, argv + argc);
	if (arguments[0].find("cmake") != string::npos) {
		// You have entered Fake CMake Mode
		vector<string> args = arguments;
		arguments = {args[0]};

		string buildDirectory = ".", sourceDirectory;
		bool isRealCMake = false;
		for (vector<string>::size_type i = 1; i < args.size(); i++) {
			string arg = args[i];
			if (arg == "--version") {
				std::cout << OSUtil::exec(FSUtil::which("cmake"), "--version").output;
				std::cout << "...except that this isn't CMake, it's "
					"Phoenix in Fake CMake Mode." << std::endl;
				return 0;
			} else if (arg == "--help") {
				OSUtil::ExecResult r = OSUtil::exec(FSUtil::which("cmake"), "--help");
				std::cout << r.output;
				std::cout << "...except that this isn't CMake, it's "
					"Phoenix in Fake CMake Mode." << std::endl;
				return r.exitcode;
			} else if (arg == "--command") {
				string cmd = args[i + 1],
					cmdArgs = cmd.substr(cmd.find(" ") + 1);
				if (cmd.find(" ") == string::npos)
					cmdArgs = "";
				cmd = cmd.substr(0, cmd.find(" "));
				return OSUtil::exec(cmd, cmdArgs, true).exitcode;
			} else if (StringUtil::startsWith(arg, "-G")) {
				// Convert CMake -G syntax to Phoenix -G syntax
				arg = arg.substr(2);
				vector<string> itms = StringUtil::split(arg, "-");
				if (itms.size() > 1) {
					// Second item, if it exists, is actually the "primary generator",
					// e.g. "-GCodeBlocks - Ninja"
					arguments.push_back("-G" + StringUtil::trim(itms[1]));
					arguments.push_back("-S" + StringUtil::trim(itms[0]));
				} else {
					// Otherwise, the first item is the primary generator,
					// e.g. "-GNinja".
					arguments.push_back("-G" + StringUtil::trim(itms[0]));
				}
			} else if (StringUtil::startsWith(arg, "-D")) {
				arg = arg.substr(2);
				vector<string> thing = StringUtil::split(arg, "=");
				thing[0] = thing[0].substr(thing[0].find(":"));
				if (thing[0] == "CMAKE_CXX_COMPILER")
					arguments.push_back("-C:C++:" + thing[1]);
			} else if (sourceDirectory.empty() || buildDirectory == ".") {
				// Assume it's a path.
				arguments.push_back(arg);
				if (sourceDirectory.length() > 0)
					buildDirectory = arg;
				else
					sourceDirectory = arg;
			} else {
				isRealCMake = true;
				// Don't break in case we get a --command.
			}
		}

		if (arguments.empty() && !isRealCMake) {
			cerr << "CMake Error: No source directory specified." << std::endl;
			NORMALLY_NOTICE;
			return 1;
		}

		// Now we check that the directories exist, and contain "Phoenixfile.phnx".
		if (!FSUtil::isDir(sourceDirectory) && !isRealCMake) {
			cerr << "CMake Error: The source directory \"" + sourceDirectory + "\" does not exist."
				<< std::endl;
			NORMALLY_NOTICE;
			return 1;
		}

		if (FSUtil::exists(FSUtil::combinePaths({sourceDirectory, "Phoenixfile.phnx"})) && !isRealCMake) {
			// All is well.
		} else if (FSUtil::exists(FSUtil::combinePaths({sourceDirectory, "CMakeLists.txt"})) || isRealCMake) {
			// Shell out to Real CMake(TM).
			args.erase(args.begin());
			return OSUtil::exec(FSUtil::which("cmake"),
				"\"" + StringUtil::join(args, "\" \"") + "\"", true).exitcode;
		} else {
			cerr << "CMake Error: The source directory \"" + sourceDirectory +
				"\" does not contain a Phoenixfile or a CMakeLists.txt." << std::endl;
			NORMALLY_NOTICE;
			return 1;
		}
	}

	return PhoenixMain(arguments);
}
