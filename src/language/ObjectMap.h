/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <map>
#include <string>

#include "Object.h"

namespace Language {

class ObjectMap : private std::map<std::string, Object*>
{
	typedef std::map<std::string, Object*> _inherited;
public:
	ObjectMap();
	~ObjectMap();

	typedef _inherited::const_iterator const_iterator;
	const_iterator begin() const { return _inherited::begin(); }
	const_iterator end() const { return _inherited::end(); }

	Object get(std::string key) const;
	Object* get_ptr(std::string key);
	void set(std::string key, Object value);

	size_type size() const { return _inherited::size(); }
};

// Convenience constructors
inline Object MapObject(ObjectMap* value)
{
	Object ret(Type::Map);
	ret.map = value;
	return ret;
}

}
