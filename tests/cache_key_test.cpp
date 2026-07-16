/**
 *
 *  @file cache_key_test.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/cgride/cache
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the LICENSE file.
 *
 *  Cgride
 *
 */
#include <cassert>
#include <string_view>
#include <unordered_set>

#include <cgride/cache/cache_key.hpp>

int main()
{
  {
    cgride::cache::CacheKey key;

    assert(key.value().empty());
    assert(key.empty());
    assert(!key.valid());
    assert(cgride::cache::to_string(key) == std::string_view(""));
    assert(key.filesystem_name().empty());
  }

  {
    cgride::cache::CacheKey key("compile:src/main.cpp");

    assert(key.value() == "compile:src/main.cpp");
    assert(!key.empty());
    assert(key.valid());
    assert(cgride::cache::to_string(key) == std::string_view("compile:src/main.cpp"));
  }

  {
    auto key = cgride::cache::CacheKey::from_string("link:app");

    assert(key.value() == "link:app");
    assert(key.valid());
  }

  {
    auto key = cgride::cache::CacheKey::from_parts({
        "compile",
        "src/main.cpp",
        "c++23",
        "debug",
    });

    assert(key.value() == "compile|src/main.cpp|c++23|debug");
    assert(key.valid());
  }

  {
    auto key = cgride::cache::CacheKey::from_parts({
        "compile",
        "part|with|pipes",
        "part%with%percent",
    });

    assert(key.value() == "compile|part%7Cwith%7Cpipes|part%25with%25percent");
  }

  {
    cgride::cache::CacheKey key("compile:src/main.cpp?profile=debug");

    assert(key.filesystem_name() == "compile_src_main.cpp_profile_debug");
  }

  {
    cgride::cache::CacheKey key("abc-DEF_123.cache");

    assert(key.filesystem_name() == "abc-DEF_123.cache");
  }

  {
    auto first = cgride::cache::CacheKey::from_string("compile:main.cpp");
    auto second = cgride::cache::CacheKey::from_string("compile:main.cpp");
    auto third = cgride::cache::CacheKey::from_string("link:app");

    assert(first == second);
    assert(first != third);
    assert(first < third);
  }

  {
    std::unordered_set<cgride::cache::CacheKey, cgride::cache::CacheKeyHash> keys;

    keys.insert(cgride::cache::CacheKey::from_string("prepare"));
    keys.insert(cgride::cache::CacheKey::from_string("compile"));
    keys.insert(cgride::cache::CacheKey::from_string("compile"));

    assert(keys.size() == 2);
    assert(keys.contains(cgride::cache::CacheKey::from_string("prepare")));
    assert(keys.contains(cgride::cache::CacheKey::from_string("compile")));
    assert(!keys.contains(cgride::cache::CacheKey::from_string("link")));
  }

  {
    cgride::cache::CacheKeyHash hash;

    auto first = hash(cgride::cache::CacheKey::from_string("compile"));
    auto second = hash(cgride::cache::CacheKey::from_string("compile"));

    assert(first == second);
  }

  return 0;
}
