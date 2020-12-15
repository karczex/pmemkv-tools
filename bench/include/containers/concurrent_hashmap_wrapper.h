// SPDX-License-Identifier: Apache-2.0
// Copyright 2021, Intel Corporation

#pragma once

#include "container_wrapper.h"
#include <libpmemobj++/container/concurrent_hash_map.hpp>
#include <libpmemobj++/container/string.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <string>

/* TODO: Remove this class when https://github.com/pmem/libpmemobj-cpp/issues/1007 implemented */

class key_equal {
public:
	template <typename M, typename U>
	bool operator()(const M &lhs, const U &rhs) const
	{
		return lhs == rhs;
	}
};

class string_hasher {
	/* hash multiplier used by fibonacci hashing */
	static const size_t hash_multiplier = 11400714819323198485ULL;

public:
	using transparent_key_equal = key_equal;

	size_t operator()(const pmem::obj::string &str) const
	{
		return hash(str.c_str(), str.size());
	}

	size_t operator()(const std::string &str) const
	{
		return hash(str.c_str(), str.size());
	}

private:
	size_t hash(const char *str, size_t size) const
	{
		size_t h = 0;
		for (size_t i = 0; i < size; ++i) {
			h = static_cast<size_t>(str[i]) ^ (h * hash_multiplier);
		}
		return h;
	}
};

class ConcurrentHashMapWrapper : public ContainerWrapper {
	using hashmap_type =
		pmem::obj::concurrent_hash_map<pmem::obj::string, pmem::obj::string, string_hasher>;

	std::string path_;
	int size_;

	struct root {
		pmem::obj::persistent_ptr<hashmap_type> map = nullptr;
	};

	pmem::obj::pool<root> pop;
	pmem::obj::persistent_ptr<root> proot = nullptr;

	const char *name_ = "pmem::obj::concurrent_hash_map<string, string>";

public:
	ConcurrentHashMapWrapper(std::string path, int size) : path_(path), size_(size)
	{
	}

	void fresh_db()
	{
		//		auto &map = pop.root() -> map;
		//		map->clear();
	}
	// XXX: change to bool
	void Open(std::string name, bool fresh_db)
	{
		try {
			pop = pmem::obj::pool<root>::open(path_, name_);
		} catch (pmem::pool_error &e) {
			try {
				pop = pmem::obj::pool<root>::create(path_, name_, size_);
			} catch (pmem::pool_error &e) {
				std::cerr << e.what() << std::endl;
				throw e;
			}
		}
		proot = pop.root();
		auto &r = proot->map;
		if (r == nullptr) {
			/* Logic when file was first opened. First, we have to
			 * allocate object of hashmap_type and attach it to the
			 * root object. */
			pmem::obj::transaction::run(pop,
						    [&] { r = pmem::obj::make_persistent<hashmap_type>(); });
		}
		r->runtime_initialize();
	}

	bool put(std::string key, std::string value)
	{

		auto &map = *(proot->map);
		try {
			map.insert_or_assign(key, value);
		} catch (pmem::transaction_error &e) {
			return false;
		}
		return true;
	}

	bool get(std::string key, std::string *value)
	{
		auto &map = pop.root()->map;
		hashmap_type::const_accessor acc;
		bool res = map->find(acc, key);
		if (res) {
			// memory leak?
			auto v = std::string(acc->second.c_str());
			value = &v;
			return true;
		}
		return false;
	}

	bool remove(std::string key)
	{
		auto &map = pop.root()->map;
		try {
			return map->erase(key);
		} catch (pmem::transaction_error &e) {
			return false;
		}
		return true;
	}

	bool close()
	{
		try {
			pop.close();
		} catch (std::logic_error &e) {
			return false;
		}
		return true;
	}

	const char *name()
	{
		return name_;
	}
};
