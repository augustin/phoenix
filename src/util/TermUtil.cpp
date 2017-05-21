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
#define PREFIX_FG "\x1B[3"
#define PREFIX_BG "\x1B[4"
#define COLOR_BLACK	 "0m"
#define COLOR_RED	 "1m"
#define COLOR_GREEN  "2m"
#define COLOR_YELLOW "3m"
#define COLOR_BLUE	 "4m"
#define COLOR_MAGENTA "5m"
#define COLOR_CYAN	 "6m"
#define COLOR_WHITE	 "7m"

#define COLORS_RESET "\x1B[0m"
#endif

using std::string;

const bool TermUtil::sIsTTY = isatty(1) && isatty(2);
#ifdef _WIN32
int TermUtil::sCurrentColor = 0x07;
int TermUtil::sCurrentBackgroundColor = 0x00;
#endif

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
	case Black:	 stream << PREFIX_FG COLOR_BLACK; break;
	case Blue:	 stream << PREFIX_FG COLOR_BLUE; break;
	case Green:  stream << PREFIX_FG COLOR_GREEN; break;
	case Cyan:   stream << PREFIX_FG COLOR_CYAN; break;
	case Red:    stream << PREFIX_FG COLOR_RED; break;
	case Purple: stream << PREFIX_FG COLOR_MAGENTA; break;
	case Yellow: stream << PREFIX_FG COLOR_YELLOW; break;
	case White:	 stream << PREFIX_FG COLOR_WHITE; break;
	case Gray:	 stream << PREFIX_FG COLOR_WHITE; break;
	}
#else
	HANDLE handle = GetStdHandle(onStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
	WORD attribute = 0;
	switch (color) {
	case Black:	 attribute = 0x00; break;
	case Blue:	 attribute = 0x01; break;
	case Green:  attribute = 0x02; break;
	case Cyan:   attribute = 0x03; break;
	case Red:    attribute = 0x04; break;
	case Purple: attribute = 0x05; break;
	case Yellow: attribute = 0x06; break;
	case White:	 attribute = 0x07; break;
	case Gray:	 attribute = 0x08; break;
	}
	sCurrentColor = attribute;
	SetConsoleTextAttribute(handle, attribute | sCurrentBackgroundColor);
#endif
}
void TermUtil::setBackgroundColor(TermColor color, bool onStdErr)
{
	if (!sIsTTY)
		return;
#ifndef _WIN32
	std::ostream& stream = onStdErr ? std::cerr : std::cout;
	switch (color) {
	case Black:	 stream << PREFIX_BG COLOR_BLACK; break;
	case Blue:	 stream << PREFIX_BG COLOR_BLUE; break;
	case Green:  stream << PREFIX_BG COLOR_GREEN; break;
	case Cyan:   stream << PREFIX_BG COLOR_CYAN; break;
	case Red:    stream << PREFIX_BG COLOR_RED; break;
	case Purple: stream << PREFIX_BG COLOR_MAGENTA; break;
	case Yellow: stream << PREFIX_BG COLOR_YELLOW; break;
	case White:	 stream << PREFIX_BG COLOR_WHITE; break;
	case Gray:	 stream << PREFIX_BG COLOR_WHITE; break;
	}
#else
	HANDLE handle = GetStdHandle(onStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
	WORD attribute = 0;
	switch (color) {
	case Black:	 attribute = 0x00; break;
	case Blue:	 attribute = 0x10; break;
	case Green:  attribute = 0x20; break;
	case Cyan:   attribute = 0x30; break;
	case Red:    attribute = 0x40; break;
	case Purple: attribute = 0x50; break;
	case Yellow: attribute = 0x60; break;
	case White:	 attribute = 0x70; break;
	case Gray:	 attribute = 0x80; break;
	}
	sCurrentBackgroundColor = attribute;
	SetConsoleTextAttribute(handle, attribute | sCurrentColor);
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
