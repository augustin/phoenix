/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Function.h"
#include "Object.h"

namespace Script {

class Stack
{
public:
	Stack();

	void push();
	void pop();

	std::map<std::string, Function> GlobalFunctions;

	inline std::vector<ObjectMap>& get() { return fStack; }

	Object get_ptr(const std::vector<std::string> variable);
	inline Object get(const std::vector<std::string> variable) {
		Object o = get_ptr(variable); if (o == nullptr) return UndefinedObject(); else return o; }
	inline Object get(const std::string variable0) {
		std::vector<std::string> variable = {variable0};
		return get(variable); }
	void set_ptr(std::vector<std::string> variable, Object value, bool forceLocal = false);
	inline void set(std::vector<std::string> variable, Object value) { set_ptr(variable, CopyObject(value)); }

	void addSuperglobal(std::string variableName, Object value);

	inline void pushDir(const std::string& dir) { fDirectoryStack.push_back(dir); }
	inline void popDir() { fDirectoryStack.pop_back(); }
	inline std::string currentDir() { return fDirectoryStack[fDirectoryStack.size() - 1]; }
	inline size_t dirDepth() { return fDirectoryStack.size(); }

	inline void appendInputFile(const std::string& path) { fInputFiles.push_back(path); }
	inline std::string currentInputFile() { return fInputFiles[fInputFiles.size() - 1]; }
	inline std::vector<std::string> inputFiles() { return fInputFiles; }

	void print();

	// Debugger hooks
	std::function<void(const std::string& path, const std::string& code,
		const uint32_t line)> mInterpreterHook;

private:
	ObjectMap fSuperglobalScope;
	std::vector<ObjectMap> fStack;
	std::vector<std::string> fDirectoryStack;

	std::vector<std::string> fInputFiles;

	std::vector<ObjectMap>::size_type getPos(std::string variable);
};

}
