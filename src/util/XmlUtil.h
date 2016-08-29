/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <vector>

class XmlGenerator
{
	typedef std::map<std::string, std::string> _tagmap;

public:
	explicit XmlGenerator(const std::string& rootTagName,
		std::map<std::string, std::string> rootTagAttrs = {}, bool pretty = true);

	void beginTag(const std::string& tagName, std::map<std::string, std::string> tagAttrs = {},
		bool oneliner = false);
	void endTag(const std::string& tagName);

	std::string finish();

private:
	std::string fBuffer;
	std::vector<std::string> fHierarchy;
	bool fPretty;

	std::string escape(const std::string& str);
	void indents();
	inline void newline() { if (fPretty) fBuffer += "\n"; }
};
