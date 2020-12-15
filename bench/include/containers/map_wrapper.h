// SPDX-License-Identifier: Apache-2.0
// Copyright 2017-2020, Intel Corporation

#pragma once

#include "container_wrapper.h"
#include <map>
#include <string>

class mapWrapper: public ContainerWrapper
{
	std::map<std::string, std::string> map;
public:
	mapWrapper()
	{
	}

	~mapWrapper()
	{
	}

	void Open(bool fresh_db, std::string name)
	{
	}

	bool put(std::string key, std::string value)
	{
		map[key] = value;
		return true;
	}

	bool get(std::string key, std::string *value)
	{
		return true;
	}

	bool remove(std::string key)
	{
		return true;
	}

	bool close()
	{
		return true;
	}
};

