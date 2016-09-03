/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "XmlUtil.h"

#include "StringUtil.h"

using std::string;

XmlGenerator::XmlGenerator(const string& rootTagName,
	std::map<string, string> rootTagAttrs, bool pretty)
	:
	fPretty(pretty)
{
	beginTag(rootTagName, rootTagAttrs);
}

void XmlGenerator::beginTag(const string& tagName, std::map<string, string> tagAttrs, bool oneliner)
{
	if (fBuffer.empty()) {
		fBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		newline();
	} else if (fHierarchy.empty()) {
		// We already finish()ed.
		return; // TODO: error reporting
	}

	indents();
	fHierarchy.push_back(tagName);
	fBuffer += "<" + tagName;
	for (_tagmap::const_iterator it = tagAttrs.begin(); it != tagAttrs.end(); it++) {
		fBuffer += " " + it->first + "=\"" + escape(it->second) + "\"";
	}
	if (oneliner) {
		fBuffer += "/>";
		fHierarchy.pop_back();
	} else
		fBuffer += ">";
	newline();
}

void XmlGenerator::endTag(const string& tagName)
{
	if (fHierarchy[fHierarchy.size() - 1] != tagName)
		return; // TODO: error reporting
	fHierarchy.pop_back();
	indents();
	fBuffer += "</" + tagName + ">";
	newline();
}

std::string XmlGenerator::finish()
{
	while (!fHierarchy.empty())
		endTag(fHierarchy[fHierarchy.size() - 1]);
	return fBuffer;
}

string XmlGenerator::escape(const string& str)
{
	string ret = str;
	StringUtil::replaceAll(ret, "&", "&amp;");
	StringUtil::replaceAll(ret, "<", "&lt;");
	StringUtil::replaceAll(ret, ">", "&gt;");
	StringUtil::replaceAll(ret, "\"", "&quot;");
	return ret;
}

void XmlGenerator::indents()
{
	if (!fPretty || fHierarchy.empty())
		return;
	for (uint32_t i = 0; i < fHierarchy.size(); i++)
		fBuffer += "\t";
}
