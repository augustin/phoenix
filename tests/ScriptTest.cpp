/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Tester.h"

#include "script/Interpreter.h"

#include "util/FSUtil.h"
#include "util/StringUtil.h"

using std::vector;
using std::string;

int main(int argc, char* argv[])
{
	vector<string> files = FSUtil::searchForFiles(FSUtil::combinePaths({__FILE__,
		"..", "script-tests"}), {".phnx"}, false);
	Tester t(true);

	const string opener = "#EXPECT: ";
	const size_t openerLen = opener.length();
	for (const string& i : files) {
		const string test = FSUtil::getContents(i);
		const string name = i.substr(i.find_last_of("/") + 1, string::npos);
		string::size_type addToLen = 0;

		if (test[0] == (char)0xEF && test[1] == (char)0xBB && test[2] == (char)0xBF
			&& test.substr(3, openerLen) == opener) {
			addToLen = 3;
		} else if (!StringUtil::startsWith(test, opener)) {
			t.result(false, i + " (failed to get expectation)");
			continue;
		}
		string expect = test.substr(openerLen + addToLen, test.find_first_of("\n") - (openerLen + addToLen));

		Script::Stack stack;
		string result;
		try {
			result = Script::Run(&stack, i)->asStringPretty();
		} catch (Script::Exception e) {
			if (expect[0] != 'E')
				e.print();
			result = "E";
			result += e.what();
		}

		bool res = (result == expect);
		t.result(res, name + (res ? "" : " (got " + result + ", expected " + expect + ")"));
	}
	return t.done();
}
