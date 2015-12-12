/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "LanguageInfo.h"

#include "language/Parser.h"

#include <string>
#include <vector>

Language::ObjectMap LanguageInfo::sData;
Language::Stack* LanguageInfo::sStack;

Language::Object* LanguageInfo::getLanguageInfo(std::string langName)
{
	Language::Object* ret = sData.get_ptr(langName);
	if (ret != nullptr)
		return ret;

	// FIXME/TODO: this is bootstrap-only Phoenix, load hardcoded location
	sData.set(langName, Language::Run(sStack, "src/data/languages/" + langName + ".phnx"));
	return sData.get_ptr(langName);
}
