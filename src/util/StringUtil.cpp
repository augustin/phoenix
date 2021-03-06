/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "StringUtil.h"

using std::string;
using std::vector;

int StringUtil::count(const string& str, const string& thing)
{
	size_t length = thing.length();
	if (length == 0) {
		return 0;
	}

	int count = 0;
	for (size_t offset = str.find(thing); offset != string::npos;
		offset = str.find(thing, offset + length)) {
        count++;
    }
	return count;
}

vector<string> StringUtil::split(const string& str, const string& delimiter)
{
	vector<string> ret(count(str, delimiter) + 1);

	size_t next = 0, oldPos = 0, newPos = 0, length = delimiter.length();
	while ((newPos = str.find(delimiter, oldPos)) != string::npos) {
		ret[next++] = str.substr(oldPos, newPos - oldPos);
		oldPos = newPos + length;
	}
	ret[next++] = str.substr(oldPos);

	return ret;
}

string StringUtil::join(const vector<string>& array, const string& delimiter,
	bool skipEmptyStrings)
{
	string ret = "";
	for (vector<string>::size_type i = 0; i < array.size(); i++) {
		if (skipEmptyStrings && array[i].empty())
			continue;
		if (i != 0)
			ret += delimiter;
		ret += array[i];
	}
	return ret;
}

string StringUtil::trim(const string& str)
{
	string::size_type from = 0;
	for (; from < str.length(); from++) {
		char c = str[from];
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;
	}
	string::size_type to = str.length() - 1;
	for (; to > from; to--) {
		char c = str[to];
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;
	}
	return str.substr(from, (to - from) + 1);
}

bool StringUtil::startsWith(const string& haystack, const string& needle)
{
	size_t haystackLen = haystack.length(), needleLen = needle.length();
	if (haystackLen < needleLen) {
		return false;
	}
	return haystack.compare(0, needleLen, needle) == 0;
}

bool StringUtil::endsWith(const string& haystack, const string& needle)
{
	size_t haystackLen = haystack.length(), needleLen = needle.length();
	if (haystackLen < needleLen) {
		return false;
	}
	return haystack.compare(haystackLen - needleLen, needleLen, needle) == 0;
}

void StringUtil::replaceAll(string& subject, const string& search, const string& replace)
{
	size_t pos = 0, searchLen = search.length();
	while ((pos = subject.find(search, pos)) != string::npos) {
		subject.replace(pos, searchLen, replace);
		pos += replace.length();
	}
}
