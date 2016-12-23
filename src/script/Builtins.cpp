/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Builtins.h"

#include <vector>

#include "Phoenix.h"

#include "Function.h"
#include "Object.h"
#include "Stack.h"

#include "util/PrintUtil.h"
#include "util/StringUtil.h"
#include "util/FSUtil.h"
#include "util/OSUtil.h"

using std::string;
using std::vector;

namespace Script {

FileBuiltin::FileBuiltin(const string& file)
	: fFile(file)
{
	fMap = new ObjectMap;

	fMap->set("exists", FunctionObject([this](Stack*, Object, ObjectMap&) -> Object {
		return BooleanObject(FSUtil::isFile(fFile));
	}));
	fMap->set("setContents", FunctionObject([this](Stack*, Object, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", zero, Type::String);
		return BooleanObject(FSUtil::setContents(fFile, zero->string));
	}));
	fMap->set("getContents", FunctionObject([this](Stack*, Object, ObjectMap&) -> Object {
		return StringObject(FSUtil::getContents(fFile));
	}));
	fMap->set("remove", FunctionObject([this](Stack*, Object, ObjectMap&) -> Object {
		return BooleanObject(FSUtil::deleteFile(fFile));
	}));
}

GlobalPhoenixObject::GlobalPhoenixObject(Stack* stack)
	:
	CObject(Type::Map)
{
	map = new ObjectMap;

	// Instantiate this object
	map->set("checkVersion", FunctionObject([](Stack*, Object, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("minimum", minimum, Type::String);
		vector<std::string> components = StringUtil::split(minimum->string, ".");
		if (components.size() == 0)
			throw Exception(Exception::TypeError,
				std::string("'minimum' must have at least 1 component"));
		vector<int> parts;
		try {
			for (const std::string& str : components)
				parts.push_back(std::stoi(str, nullptr, 10));
		} catch (...) {
			throw Exception(Exception::SyntaxError, "expected an integer in call to checkVersion");
		}
		while (parts.size() != 3)
			parts.push_back(-1);
		if (parts[0] > PHOENIX_VERSION_MAJOR ||
			parts[1] > PHOENIX_VERSION_MINOR ||
			parts[2] > PHOENIX_VERSION_PATCH)
			throw Exception(Exception::UserError,
				std::string("minimum version of Phoenix required is ").append(minimum->string)
				 .append(" and this is Phoenix " PHOENIX_VERSION));
		return UndefinedObject();
	}));

	// Also create $$OS superglobals
	stack->addSuperglobal("OS", StringObject(OSUtil::name()));
	stack->addSuperglobal(OSUtil::name(false), BooleanObject(true));
	if (OSUtil::isFamilyUnix())
		stack->addSuperglobal("UNIX", BooleanObject(true));

	// Also instantiate the GlobalFunctions object
	stack->GlobalFunctions.insert({"Map", Function([](Stack*, Object, ObjectMap& params) -> Object {
		return MapObject(new ObjectMap(params));
	})});

	stack->GlobalFunctions.insert({"print", Function([](Stack*, Object, ObjectMap& params) -> Object {
		Object zero = params.get("0");
		PrintUtil::message(zero->asStringRaw());
		return UndefinedObject();
	})});
	stack->GlobalFunctions.insert({"dump", Function([](Stack*, Object, ObjectMap& params) -> Object {
		Object zero = params.get("0");
		PrintUtil::message(zero->asStringPretty());
		return UndefinedObject();
	})});
	stack->GlobalFunctions.insert({"fatal", Function([](Stack*, Object, ObjectMap& params) -> Object {
		Script::Object zero = params.get("0");
		throw Exception(Exception::UserError, zero->asStringRaw());
	})});

	stack->GlobalFunctions.insert({"parseInt", Function([](Stack*, Object, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", zero, Type::String);
		try {
			return IntegerObject(std::stoi(zero->string));
		} catch (...) {
			return UndefinedObject();
		}
	})});

	stack->GlobalFunctions.insert({"File", Function([](Stack* stack, Object, ObjectMap& params) -> Object {
		NativeFunction_COERCE_OR_THROW("0", zero, Type::String);
		std::string file = FSUtil::normalizePath(zero->string);
		if (!FSUtil::isPathAbsolute(file))
			file = FSUtil::combinePaths({stack->currentDir(), file});
		return Script::MapObject((new FileBuiltin(file))->fMap);
	})});
}

}
