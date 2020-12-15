// SPDX-License-Identifier: Apache-2.0
// Copyright 2017-2020, Intel Corporation

#pragma once

#include <memory>
#include <string>

#include "container_wrapper.h"
#include "libpmemkv.hpp"

class pmemkvWrapper : public ContainerWrapper {
	std::unique_ptr<pmem::kv::db> kv_;
	std::string engine_;
	std::string path_;
	int size_;
	const char *name_ = "pmemkv";

public:
	// XXX change name to empty_container
	void fresh_db()
	{
		kv_.reset();
	}

	pmemkvWrapper(std::string engine, std::string path, int size)
	    : engine_(engine), path_(path), size_(size)
	{
	}

	void Open(std::string name, bool fresh_db)
	{

		pmem::kv::config cfg;
		auto cfg_s = cfg.put_string("path", path_);

		if (cfg_s != pmem::kv::status::OK)
			throw std::runtime_error("putting 'path' to config failed");

		if (fresh_db) {
			cfg_s = cfg.put_uint64("force_create", 1);
			if (cfg_s != pmem::kv::status::OK) {
				throw std::runtime_error("putting 'force_create' to config failed");
			}
			cfg_s = cfg.put_uint64("size", size_);

			if (cfg_s != pmem::kv::status::OK) {
				throw std::runtime_error("putting 'size' to config failed");
			}
			kv_.reset();
			/* Remove pool file. This should be
			 * implemented using libpmempool for backward
			 * compatibility. */
			if (pmempool_rm(path_.data(), PMEMPOOL_RM_FORCE) != 0) {
				throw std::runtime_error("Cannot remove pool: " + path_);
			}

		}
		if (!kv_) {
			kv_ = std::unique_ptr<pmem::kv::db>(new pmem::kv::db);
		}
		auto s = kv_->open(engine_, std::move(cfg));

		if (s != pmem::kv::status::OK) {

			throw std::runtime_error("Cannot start engine" + pmem::kv::errormsg());
		}

		//			fprintf(stderr,
		//				"Cannot start engine (%s) for path (%s) with %i GB
		//capacity\n%s\n\nUSAGE: %s", 				engine_, FLAGS_db, FLAGS_db_size_in_gb,
		//pmem::kv::errormsg().c_str(), 				USAGE.c_str());
	}

	bool put(std::string key, std::string value)
	{
		return kv_->put(key, value) == pmem::kv::status::OK;
	}

	bool get(std::string key, std::string *value)
	{
		return kv_->get(key, value) == pmem::kv::status::OK;
	}

	bool remove(std::string key)
	{
		return kv_->remove(key) == pmem::kv::status::OK;
	}

	bool close()
	{
		kv_->close();
		return true;
	}

	const char *name()
	{
		return name_;
	}
};
