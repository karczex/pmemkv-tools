// SPDX-License-Identifier: Apache-2.0
// Copyright 2017-2020, Intel Corporation

#pragma once

#include "container_wrapper.h"
#include <map>
#include <memory>
#include <string>

class mapWrapper : public ContainerWrapper {
	std::map<std::string, std::string> map;
	const char *name_ = "std::map";

public:
	void Open(std::string name, bool fresh_db)
	{
	}

	bool put(std::string key, std::string value)
	{
		map[key] = value;
		return true;
	}

	bool get(std::string key, std::string *value)
	{
		auto it = map.find(key);
		if (it == map.end())
		{
			return false;
		}
		value = &it->second;

		return true;
	}

	bool remove(std::string key)
	{
		return map.erase(key);
	}

	bool close()
	{
		return true;
	}

	const char *name()
	{
		return name_;
	}
};
