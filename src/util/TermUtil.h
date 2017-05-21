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

	static void setColor(TermColor color, bool onStdErr = false);
	static void setBackgroundColor(TermColor color, bool onStdErr = false);
	static void resetColors(bool onStdErr = false);

private:
	static const bool sIsTTY;
#ifdef _WIN32
	static int sCurrentColor;
	static int sCurrentBackgroundColor;
#endif
};
