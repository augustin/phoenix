/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "OSUtil.h"

#include <stdio.h>

using std::string;

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

OSUtil::ExecResult OSUtil::exec(const string& command)
{
	std::string cmd = command;
	cmd.append(" 2>&1");

	ExecResult ret;
	FILE* proc = popen(cmd.c_str(), "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), proc) != 0)
		ret.output.append(buf);
	ret.exitcode = pclose(proc);
	return ret;
}
