/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "OSUtil.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

using std::string;

#ifdef _MSC_VER
#  define popen _popen
#  define pclose _pclose
#endif

string OSUtil::name(bool userfriendly)
{
#if defined(_WIN32)
	return userfriendly ? "Windows" : "WIN32";
#elif defined(__linux__)
	return userfriendly ? "Linux" : "LINUX";
#elif defined(__FreeBSD__)
	return userfriendly ? "FreeBSD" : "FREEBSD";
#elif defined(__HAIKU__)
	return userfriendly ? "Haiku" : "HAIKU";
#elif defined(__APPLE__)
	return userfriendly ? "Apple" : "APPLE";
#else
#  error "Unknown OS (::name)."
#endif
}

bool OSUtil::isFamilyUnix()
{
#if defined(_WIN32)
	return false;
#elif (defined(__linux__) || defined(__FreeBSD__) || defined(__HAIKU__) \
  || defined(__APPLE__))
	return true;
#else
#  error "Unknown OS (::isUnixFamily)."
#endif
}

OSUtil::ExecResult OSUtil::exec(const string& program, const string& args, bool forwardOutput)
{
	string cmd = "\"" + program + "\"" + " " + args + " 2>&1";
#ifdef _WIN32
	cmd = "\"" + cmd + "\"";
#endif

	ExecResult ret;
	FILE* proc = ::popen(cmd.c_str(), "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), proc) != 0) {
		ret.output.append(buf);
		if (forwardOutput)
			std::cout << buf << std::flush;
	}
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
