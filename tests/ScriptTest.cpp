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
		"..", "script-tests"}), {""}, false);
	Tester t(true);

	// Global function instatiation (copied from GlobalLanguageObject.cpp)
	Script::GlobalFunctions.insert({"Map", Script::Function([](Script::Stack*,
			Script::Object*, Script::ObjectMap& params) -> Script::Object {
		return Script::MapObject(new Script::ObjectMap(params));
	})});

	const string opener = "#EXPECT: ";
	const size_t openerLen = opener.length();
	for (const string& i : files) {
		const string test = FSUtil::getContents(i);
		const string name = i.substr(i.find_last_of("/") + 1, string::npos);

		if (!StringUtil::startsWith(test, opener)) {
			t.result(false, name + " (failed to get expectation)");
			continue;
		}
		string expect = test.substr(openerLen, test.find_first_of("\n") - openerLen);

		Script::Stack stack;
		Script::Object result;
		bool threwException = false;
		Script::Exception exceptionResult(Script::Exception::InternalError, "");
		try {
			result = Script::Run(&stack, i);
		} catch (Script::Exception e) {
			threwException = true;
			exceptionResult = e;
		}

		if (threwException) {
			if (expect[0] != 'E') {
				t.result(false, name + " (unexpected exception)");
				exceptionResult.print();
			} else
				t.result((exceptionResult.what() == expect.substr(1, string::npos)), name);
			continue;
		} else {
			bool res = (result.asStringPretty() == expect);
			if (res)
				t.result(true, name);
			else
				t.result(false, name + " (got " + result.asStringPretty() + ", expected " + expect + ")");
		}
	}
	return t.done();
}
