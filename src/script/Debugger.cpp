/*
 * (C) 2015-2017 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "script/Debugger.h"

#include <iostream>

#include "script/Interpreter.h"
#include "util/StringUtil.h"
#include "util/TermUtil.h"

using std::string;
using std::vector;

namespace Script {

static void DrawVerticalLine(TermUtil& tu, int x, int startY, int height)
{
	for (int y = startY; y < height; y++) {
		tu.moveCursorTo(x, y);
		tu.write(" ");
	}
}
static void DrawHorizontalLine(TermUtil& tu, int startX, int y, int width)
{
	tu.moveCursorTo(startX, y);
	tu.write(string(width, ' '));
}

Object Debugger(Stack* stack, std::string path)
{
	TermUtil tu;
	tu.startup();

	std::pair<int /*width*/, int /*height*/> size = tu.size();
	int codeview_height, codeview_width;
	codeview_height = size.second * 2/3;
	codeview_width = size.first * 2/3;

	std::string runningPath;
	std::string runningCode;
	int runningLine = 0;

	auto DrawBorders = [&]() -> void {
		tu.setBackgroundColor(TermUtil::Yellow);
		tu.setColor(TermUtil::Black);
		DrawHorizontalLine(tu, 0, codeview_height, size.first);
		DrawVerticalLine(tu, codeview_width, 0, codeview_height);
		DrawHorizontalLine(tu, 0, 0, size.first);
		tu.moveCursorTo(1, 0);
		tu.write("Source");
		tu.moveCursorTo(codeview_width + 1, 0);
		tu.write("Variables");
		tu.resetColors();
	}; DrawBorders();
	auto DrawCode = [&]() -> void {
		const int startLine = std::max(runningLine - (codeview_height / 2), 1);
		const int width = std::max(std::to_string(startLine).size(),
			std::to_string(startLine + codeview_height).size()) + 1;

		// Draw the left-hand bar
		tu.setBackgroundColor(TermUtil::Gray);
		tu.setColor(TermUtil::Black);
		for (int y = 1; y < codeview_height; y++) {
			const int line = startLine + (y - 1);
			tu.moveCursorTo(0, y);
			string lineno = std::to_string(line);
			tu.write(lineno + string(width - lineno.size() -
				(line == runningLine ? 1 : 0), ' '));
			if (line == runningLine) {
				tu.write(">");
			}
		}
		tu.resetColors();

		// Draw the actual code
		vector<string> lines = StringUtil::split(runningCode, "\n");
		const int end = (codeview_width - width);
		for (int y = 1; y < codeview_height; y++) {
			const int line = startLine + (y - 1);
			// First, clear the line completely
			tu.moveCursorTo(width, y);
			tu.write(string(end, ' '));
			// Then add the syntax-highlighted source
			tu.moveCursorTo(width, y);
			const string& linestr = lines[line - 1];
			for (string::size_type i = 0; i < end && i < linestr.size(); i++) {
				char c = linestr[i];
				bool resetAfter = false;
				switch (c) {
				case '\t':
					c = ' '; // FIXME: should be 4 spaces
				break;
				case '#':
					tu.setColor(TermUtil::Gray);
					tu.write(linestr.substr(i, end - i));
					i = end;
					continue;
				case '(': case ')': case '[': case ']': case '=': case ',': case '.':
				case ';': case ':': case '&': case '|': case '/': case '*': case '+':
				case '-':
					tu.setColor(TermUtil::Cyan);
					resetAfter = true;
				break;
				case '"': case '\'':
					tu.setColor(TermUtil::Gray);
					tu.write(c);
					i++;
					for (; linestr[i] != c && i < end; i++) {
						tu.write(linestr[i]);
						if (linestr[i] == '\\') {
							tu.write(linestr[i + 1]);
							i++;
						}
					}
					resetAfter = true;
				break;
				}
				tu.write(c);
				if (resetAfter)
					tu.resetColors();
			}
			tu.resetColors();
		}
	};
	auto DrawStack = [&]() {
		vector<ObjectMap>& stk = stack->get();
		int y = 1;
		int end = size.first - (codeview_width + 1);
		for (vector<ObjectMap>::const_reverse_iterator m = stk.rbegin(); m != stk.rend(); m++) {
			for (ObjectMap::const_iterator it = m->begin(); it != m->end() && y < codeview_height; it++) {
				tu.moveCursorTo(codeview_width + 1, y);
				tu.write(it->first.substr(0, end));
				string val = it->second->asStringRaw();
				if ((it->first.size() + 3) < end) {
					int x = codeview_width + 1 + it->first.size() + 1;
					tu.moveCursorTo(x, y);
					tu.write(val.substr(0, size.first - x));
				}
				y++;
			}
		}
	};

	stack->mInterpreterHook = [&](const std::string& path, const std::string& code,
			const uint32_t line) -> void {
		runningPath = path;
		runningCode = code;
		runningLine = line;
		DrawCode();
		DrawStack();

		std::string str;
		std::getline(std::cin, str);
	};

	stack->set({"Thing"}, IntegerObject(6));
	Script::Run(stack, path);

	tu.shutdown();
	exit(0);
	//return Run(stack, path);
}

}
