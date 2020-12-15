// SPDX-License-Identifier: Apache-2.0
// Copyright 2017-2020, Intel Corporation

#pragma once

#include <string>

class ContainerWrapper {
public:
	virtual void Open(std::string name, bool fresh_db) = 0;

	virtual bool put(std::string key, std::string value) = 0;

	virtual bool get(std::string key, std::string *value) = 0;

	virtual bool remove(std::string key) = 0;

	virtual bool close() = 0;

	virtual const char *name() = 0;
};
