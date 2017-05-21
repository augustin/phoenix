/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "TermUtil.h"

#include <iostream>

#ifdef _WIN32
#  include <windows.h>
#  ifndef _MSC_VER
#    include <unistd.h>
#  else
#    include <io.h>
#  endif
#endif

#ifndef _WIN32
// Escape code constants
#define COLOR_BLACK	"\x1B[30m"
#define COLOR_RED	"\x1B[31m"
#define COLOR_GREEN "\x1B[32m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_BLUE	"\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN	"\x1B[36m"
#define COLOR_WHITE	"\x1B[37m"

#define COLORS_RESET "\x1B[0m"
#endif

using std::string;

const bool TermUtil::sIsTTY = isatty(1) && isatty(2);

void TermUtil::init()
{
	if (!sIsTTY)
		return;

	// Disable standard I/O buffering
#ifndef _WIN32
	setvbuf(stdin, (char *)NULL, _IOLBF, BUFSIZ);
    setvbuf(stdout, (char *)NULL, _IOLBF, BUFSIZ);
#else
	setvbuf(stdin, (char *)NULL, _IONBF, BUFSIZ);
    setvbuf(stdout, (char *)NULL, _IONBF, BUFSIZ);
#endif
}

void TermUtil::setColor(TermColor color, bool onStdErr)
{
	if (!sIsTTY)
		return;
#ifndef _WIN32
	std::ostream& stream = onStdErr ? std::cerr : std::cout;
	switch (color) {
	case Black:	 stream << COLOR_BLACK; break;
	case Blue:	 stream << COLOR_BLUE; break;
	case Green:  stream << COLOR_GREEN; break;
	case Cyan:   stream << COLOR_CYAN; break;
	case Red:    stream << COLOR_RED; break;
	case Purple: stream << COLOR_MAGENTA; break;
	case Yellow: stream << COLOR_YELLOW; break;
	case White:	 stream << COLOR_WHITE; break;
	case Gray:	 stream << COLOR_WHITE; break;
	}
#else
	HANDLE handle = GetStdHandle(onStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
	WORD attributes = 0;
	switch (color) {
	case Black:	 attributes |= 0x00; break;
	case Blue:	 attributes |= 0x01; break;
	case Green:  attributes |= 0x02; break;
	case Cyan:   attributes |= 0x03; break;
	case Red:    attributes |= 0x04; break;
	case Purple: attributes |= 0x05; break;
	case Yellow: attributes |= 0x06; break;
	case White:	 attributes |= 0x07; break;
	case Gray:	 attributes |= 0x08; break;
	}
	SetConsoleTextAttribute(handle, attributes);
#endif
}
void TermUtil::resetColors(bool onStdErr)
{
	if (!sIsTTY)
		return;
#ifndef _WIN32
	(onStdErr ? std::cerr : std::cout) << COLORS_RESET;
#else
	HANDLE handle = GetStdHandle(onStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, 0x07);
#endif
}
