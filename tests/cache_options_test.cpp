/**
 *
 *  @file cache_options_test.cpp
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
#include <filesystem>

#include <cgride/cache/cache_options.hpp>

int main()
{
  {
    cgride::cache::CacheOptions options;

    assert(options.root() == std::filesystem::path(".cgride/cache"));
    assert(options.entries_directory() == std::filesystem::path(".cgride/cache") / "entries");
    assert(options.objects_directory() == std::filesystem::path(".cgride/cache") / "objects");
    assert(options.enabled());
    assert(options.verify_outputs());
    assert(!options.hash_inputs());
    assert(options.create_directories());
    assert(options.valid());
  }

  {
    cgride::cache::CacheOptions options("build/cache");

    assert(options.root() == std::filesystem::path("build/cache"));
    assert(options.entries_directory() == std::filesystem::path("build/cache") / "entries");
    assert(options.objects_directory() == std::filesystem::path("build/cache") / "objects");
    assert(options.enabled());
    assert(options.valid());
  }

  {
    auto options = cgride::cache::CacheOptions::defaults();

    assert(options.root() == std::filesystem::path(".cgride/cache"));
    assert(options.enabled());
    assert(options.verify_outputs());
    assert(!options.hash_inputs());
    assert(options.create_directories());
    assert(options.valid());
  }

  {
    auto options = cgride::cache::CacheOptions::disabled();

    assert(!options.enabled());
    assert(options.valid());
  }

  {
    cgride::cache::CacheOptions options;

    options
        .root("custom-cache")
        .enabled(false)
        .verify_outputs(false)
        .hash_inputs(true)
        .create_directories(false);

    assert(options.root() == std::filesystem::path("custom-cache"));
    assert(options.entries_directory() == std::filesystem::path("custom-cache") / "entries");
    assert(options.objects_directory() == std::filesystem::path("custom-cache") / "objects");
    assert(!options.enabled());
    assert(!options.verify_outputs());
    assert(options.hash_inputs());
    assert(!options.create_directories());
    assert(options.valid());
  }

  {
    cgride::cache::CacheOptions options;

    options.root({});

    assert(options.root().empty());
    assert(!options.valid());

    options.enabled(false);

    assert(options.valid());
  }

  {
    cgride::cache::CacheOptions options;

    options
        .enabled(true)
        .root("cache-root")
        .verify_outputs(true)
        .hash_inputs(true)
        .create_directories(true);

    assert(options.enabled());
    assert(options.root() == std::filesystem::path("cache-root"));
    assert(options.verify_outputs());
    assert(options.hash_inputs());
    assert(options.create_directories());
    assert(options.valid());
  }

  return 0;
}
