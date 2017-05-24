/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "TermUtil.h"

#include <iostream>

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#else
#  include <unistd.h>
#  include <sys/ioctl.h>
#endif

#ifndef _WIN32
// Universal ANSI codes
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

// xterm-specific codes
// If anyone really wants to use a non-xterm-compatible terminal,
// these should get refactored into a struct mapping of some kind
// (termbox uses a system like this.)
#define XTERM_SWITCH_TO_ALTERNATE_SCREEN	"\x1B[?1049h"
#define XTERM_SWITCH_TO_REGULAR_SCREEN		"\x1B[?1049l"
#endif

using std::string;

const bool TermUtil::sIsTTY = isatty(1) && isatty(2);
bool TermUtil::fStarted = false;
#ifdef _WIN32
int TermUtil::sCurrentColor = 0x07;
int TermUtil::sCurrentBackgroundColor = 0x00;
void* TermUtil::fConsoleScreenBuffer = GetStdHandle(STD_OUTPUT_HANDLE);
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

void TermUtil::setColor(TermColor color)
{
	if (!sIsTTY)
		return;
#ifndef _WIN32
	switch (color) {
	case Black:	 std::cout << PREFIX_FG COLOR_BLACK; break;
	case Blue:	 std::cout << PREFIX_FG COLOR_BLUE; break;
	case Green:  std::cout << PREFIX_FG COLOR_GREEN; break;
	case Cyan:   std::cout << PREFIX_FG COLOR_CYAN; break;
	case Red:    std::cout << PREFIX_FG COLOR_RED; break;
	case Purple: std::cout << PREFIX_FG COLOR_MAGENTA; break;
	case Yellow: std::cout << PREFIX_FG COLOR_YELLOW; break;
	case White:	 std::cout << PREFIX_FG COLOR_WHITE; break;
	case Gray:	 std::cout << PREFIX_FG COLOR_WHITE; break;
	}
#else
	WORD attribute = 0;
	switch (color) {
	case Black:	 attribute = 0x00; break;
	case Blue:	 attribute = 0x09; break;
	case Green:  attribute = 0x0A; break;
	case Cyan:   attribute = 0x0B; break;
	case Red:    attribute = 0x0C; break;
	case Purple: attribute = 0x0D; break;
	case Yellow: attribute = 0x0E; break;
	case White:	 attribute = 0x07; break;
	case Gray:	 attribute = 0x08; break;
	}
	sCurrentColor = attribute;
	SetConsoleTextAttribute(fConsoleScreenBuffer, attribute | sCurrentBackgroundColor);
#endif
}
void TermUtil::setBackgroundColor(TermColor color)
{
	if (!sIsTTY)
		return;
#ifndef _WIN32
	switch (color) {
	case Black:	 std::cout << PREFIX_BG COLOR_BLACK; break;
	case Blue:	 std::cout << PREFIX_BG COLOR_BLUE; break;
	case Green:  std::cout << PREFIX_BG COLOR_GREEN; break;
	case Cyan:   std::cout << PREFIX_BG COLOR_CYAN; break;
	case Red:    std::cout << PREFIX_BG COLOR_RED; break;
	case Purple: std::cout << PREFIX_BG COLOR_MAGENTA; break;
	case Yellow: std::cout << PREFIX_BG COLOR_YELLOW; break;
	case White:	 std::cout << PREFIX_BG COLOR_WHITE; break;
	case Gray:	 std::cout << PREFIX_BG COLOR_WHITE; break;
	}
#else
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
	SetConsoleTextAttribute(fConsoleScreenBuffer, attribute | sCurrentColor);
#endif
}
void TermUtil::resetColors()
{
	if (!sIsTTY)
		return;
#ifndef _WIN32
	std::cout << COLORS_RESET;
#else
	sCurrentBackgroundColor = 0x0;
	sCurrentColor = 0x7;
	SetConsoleTextAttribute(fConsoleScreenBuffer, sCurrentBackgroundColor | sCurrentColor);
#endif
}

TermUtil::TermUtil()
{
}
TermUtil::~TermUtil()
{
	shutdown();
}

void TermUtil::startup()
{
	if (fStarted)
		return;
	fStarted = true;

#ifndef WIN32
	std::cout << XTERM_SWITCH_TO_ALTERNATE_SCREEN << std::flush;
#else
	fConsoleScreenBuffer = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(fConsoleScreenBuffer);
#endif
	moveCursorTo(0, 0);
}

void TermUtil::shutdown()
{
	if (!fStarted)
		return;
	fStarted = false;

#ifndef WIN32
	std::cout << XTERM_SWITCH_TO_REGULAR_SCREEN << std::flush;
#else
	SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
	CloseHandle(fConsoleScreenBuffer);
	fConsoleScreenBuffer = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

std::pair<int, int> TermUtil::size()
{
#ifndef _WIN32
	struct winsize sz;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &sz) == 0)
		return {sz.ws_col, sz.ws_row};
	return {-1, -1};
#else
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(fConsoleScreenBuffer, &info);
	return {(info.srWindow.Right - info.srWindow.Left) + 1,
		(info.srWindow.Bottom - info.srWindow.Top) + 1};
#endif
}

void TermUtil::moveCursorTo(int x, int y)
{
#ifndef _WIN32
	std::cout << "\x1B[" << (y + 1) << ";" << (x + 1) << "H";
#else
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(fConsoleScreenBuffer, pos);
#endif
}
void TermUtil::write(const std::string& str)
{
#ifndef _WIN32
	std::cout << str;
#else
	WriteConsole(fConsoleScreenBuffer, str.c_str(), str.length(), nullptr, nullptr);
#endif
}
void TermUtil::write(char c)
{
	write(string(1, c));
}
