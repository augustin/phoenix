/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <string>

class TermUtil
{
public:
	enum TermColor {
		Black = 0,
		Blue,
		Green,
		Cyan,
		Red,
		Purple,
		Yellow,
		White,
		Gray,
	};

	static void init();

	static void setColor(TermColor color);
	static void setBackgroundColor(TermColor color);
	static void resetColors();

	TermUtil();
	~TermUtil();

	void startup();
	void shutdown();

	std::pair<int, int> size();

	void moveCursorTo(int x, int y);
	void write(char c);
	void write(const std::string& str);

private:
	static const bool sIsTTY;
	static bool fStarted;
#ifdef _WIN32
	static int sCurrentColor;
	static int sCurrentBackgroundColor;
	static void* fConsoleScreenBuffer;
#endif
};
