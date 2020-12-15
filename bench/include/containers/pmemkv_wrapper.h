// SPDX-License-Identifier: Apache-2.0
// Copyright 2017-2020, Intel Corporation

#pragma once


#include <string>

#include "container_wrapper.h"
#include "libpmemkv.hpp"

class pmemkvWrapper: public ContainerWrapper
{
	pmem::kv::db kv_;
	pmem::kv::config cfg;
	std::string engine_;

	void fresh_db()
	{

	}
public:
	pmemkvWrapper(std::string engine, std::string path, int size)
	{
		engine_ = engine;
		auto cfg_s = cfg.put_string("path", path);

		if (cfg_s != pmem::kv::status::OK)
			throw std::runtime_error("putting 'path' to config failed");

		cfg_s = cfg.put_uint64("force_create", 1);
		if (cfg_s != pmem::kv::status::OK)
			throw std::runtime_error(
				"putting 'force_create' to config failed");

		cfg_s = cfg.put_uint64("size", size);

		if (cfg_s != pmem::kv::status::OK)
			throw std::runtime_error(
				"putting 'size' to config failed");

	// XXX if freshdb
		if (size > 0) {
//			auto start = g_env->NowMicros();
			std::remove(path.data());
			}
	}

	void Open(bool fresh_db, std::string name)
	{
		auto s = kv_.open(engine_, std::move(cfg));

		if (s !=  pmem::kv::status::OK) {
//			fprintf(stderr,
//				"Cannot start engine (%s) for path (%s) with %i GB capacity\n%s\n\nUSAGE: %s",
//				engine_, FLAGS_db, FLAGS_db_size_in_gb, pmem::kv::errormsg().c_str(),
//				USAGE.c_str());
			exit(-42);
		}
	}

	bool put(std::string key, std::string value)
	{
		return kv_.put(key, value) == pmem::kv::status::OK;
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

