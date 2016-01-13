/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "OSUtil.h"

#include <stdio.h>
#include <stdlib.h>

using std::string;

#ifdef _MSC_VER
#  define popen _popen
#  define pclose _pclose
#endif

OSUtil::ExecResult OSUtil::exec(const string& program, const string& args)
{
	string cmd = "\"" + program + "\"" + " " + args + " 2>&1";

	ExecResult ret;
	FILE* proc = ::popen(cmd.c_str(), "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), proc) != 0)
		ret.output.append(buf);
	ret.exitcode = ::pclose(proc);
	return ret;
}

string OSUtil::getEnv(const string& env)
{
	string ret;
	const char* gotenv = ::getenv(env.c_str());
	if (gotenv != nullptr)
		ret = string(gotenv);
	return ret;
}
