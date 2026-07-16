/**
 *
 *  @file cache_store_test.cpp
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
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <cgride/cache/cache_store.hpp>

namespace
{
  [[nodiscard]] std::filesystem::path make_test_directory()
  {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();

    auto directory = std::filesystem::temp_directory_path() /
                     ("cgride_cache_store_test_" + std::to_string(now));

    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    return directory;
  }

  void write_text_file(const std::filesystem::path &path, const std::string &content)
  {
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << content;
  }

  [[nodiscard]] bool contains_key(
      const std::vector<cgride::cache::CacheKey> &keys,
      const cgride::cache::CacheKey &key)
  {
    for (const auto &candidate : keys)
    {
      if (candidate == key)
      {
        return true;
      }
    }

    return false;
  }

} // namespace

int main()
{
  {
    cgride::cache::CacheStore store;

    assert(store.options().root() == std::filesystem::path(".cgride/cache"));
    assert(store.options().enabled());
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    assert(store.options().root() == directory / "cache");

    cgride::cache::CacheOptions other(directory / "other-cache");

    store.options(other);

    assert(store.options().root() == directory / "other-cache");

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto result = store.initialize();

    assert(result);
    assert(std::filesystem::exists(options.entries_directory()));
    assert(std::filesystem::exists(options.objects_directory()));
    assert(std::filesystem::is_directory(options.entries_directory()));
    assert(std::filesystem::is_directory(options.objects_directory()));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    options.enabled(false);

    cgride::cache::CacheStore store(options);

    auto result = store.initialize();

    assert(result);
    assert(!std::filesystem::exists(options.entries_directory()));
    assert(!std::filesystem::exists(options.objects_directory()));

    std::filesystem::remove_all(directory);
  }

  {
    cgride::cache::CacheOptions options;
    options.root({});

    cgride::cache::CacheStore store(options);

    auto result = store.initialize();

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Cannot initialize cache with invalid options.");
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto key = cgride::cache::CacheKey::from_string("compile:src/main.cpp");

    assert(store.entry_path(key) ==
           options.entries_directory() / "compile_src_main.cpp.entry");

    assert(store.object_directory(key) ==
           options.objects_directory() / "compile_src_main.cpp");

    assert(!store.contains(key));
    assert(!store.contains(cgride::cache::CacheKey{}));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    cgride::cache::CacheEntry entry;

    auto result = store.put(entry);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Cannot store an invalid cache entry.");

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto key = cgride::cache::CacheKey::from_string("compile:main.cpp");

    auto created_at = cgride::cache::CacheEntry::TimePoint(
        std::chrono::milliseconds{12345});

    auto last_used_at = cgride::cache::CacheEntry::TimePoint(
        std::chrono::milliseconds{67890});

    auto input_path = directory / "src" / "main.cpp";
    auto output_path = directory / "build" / "main.o";

    write_text_file(input_path, "int main() { return 0; }\n");
    write_text_file(output_path, "object\n");

    auto signature = cgride::cache::FileSignature::from_file(input_path, true);

    assert(signature);

    cgride::cache::CacheEntry entry(key);

    entry
        .producer("compile:main.cpp")
        .created_at(created_at)
        .last_used_at(last_used_at)
        .hit_count(3)
        .input(signature.value())
        .output(output_path);

    auto put = store.put(entry);

    assert(put);
    assert(store.contains(key));
    assert(std::filesystem::exists(store.entry_path(key)));

    auto loaded = store.get(key);

    assert(loaded);

    auto value = loaded.value();

    assert(value.key() == key);
    assert(value.producer() == "compile:main.cpp");
    assert(value.created_at() == created_at);
    assert(value.has_last_used_at());
    assert(value.last_used_at().value() == last_used_at);
    assert(value.hit_count() == 3);

    assert(value.inputs().size() == 1);
    assert(value.inputs()[0].path() == input_path);
    assert(value.inputs()[0].has_size());
    assert(value.inputs()[0].size().value() == signature.value().size().value());
    assert(value.inputs()[0].has_last_write_time());
    assert(value.inputs()[0].has_content_hash());
    assert(value.inputs()[0].content_hash().value() == signature.value().content_hash().value());

    assert(value.outputs().size() == 1);
    assert(value.outputs()[0] == output_path);
    assert(value.outputs_exist());
    assert(value.inputs_match(true));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto key = cgride::cache::CacheKey::from_string("missing");

    auto result = store.get(key);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::NotFound);
    assert(result.error().message() == "Cache entry was not found.");
    assert(result.error().path().has_value());
    assert(result.error().path().value() == store.entry_path(key));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto result = store.get(cgride::cache::CacheKey{});

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Cannot read cache entry with an invalid key.");

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    options.enabled(false);

    cgride::cache::CacheStore store(options);

    auto key = cgride::cache::CacheKey::from_string("compile");

    cgride::cache::CacheEntry entry(key);

    auto put = store.put(entry);

    assert(put);
    assert(!store.contains(key));

    auto get = store.get(key);

    assert(!get);
    assert(get.error().code() == cgride::core::ErrorCode::InvalidState);
    assert(get.error().message() == "Cannot read cache entry while cache is disabled.");

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto first = cgride::cache::CacheKey::from_string("compile:a.cpp");
    auto second = cgride::cache::CacheKey::from_string("compile:b.cpp");

    assert(store.put(cgride::cache::CacheEntry(first)));
    assert(store.put(cgride::cache::CacheEntry(second)));

    auto result = store.keys();

    assert(result);

    auto keys = result.value();

    assert(keys.size() == 2);
    assert(contains_key(keys, first));
    assert(contains_key(keys, second));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto key = cgride::cache::CacheKey::from_string("remove-me");

    assert(store.put(cgride::cache::CacheEntry(key)));
    assert(store.contains(key));

    auto object_directory = store.object_directory(key);
    write_text_file(object_directory / "output.o", "object");

    assert(std::filesystem::exists(object_directory));

    auto result = store.remove(key);

    assert(result);
    assert(!store.contains(key));
    assert(!std::filesystem::exists(store.entry_path(key)));
    assert(!std::filesystem::exists(object_directory));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto first = cgride::cache::CacheKey::from_string("first");
    auto second = cgride::cache::CacheKey::from_string("second");

    assert(store.put(cgride::cache::CacheEntry(first)));
    assert(store.put(cgride::cache::CacheEntry(second)));

    assert(store.contains(first));
    assert(store.contains(second));

    auto result = store.clear();

    assert(result);
    assert(!store.contains(first));
    assert(!store.contains(second));
    assert(std::filesystem::exists(options.entries_directory()));
    assert(std::filesystem::exists(options.objects_directory()));

    auto keys = store.keys();

    assert(keys);
    assert(keys.value().empty());

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheOptions options(directory / "cache");
    cgride::cache::CacheStore store(options);

    auto key = cgride::cache::CacheKey::from_string("broken");

    assert(store.initialize());

    write_text_file(store.entry_path(key), "not-a-valid-entry\n");

    auto result = store.get(key);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Invalid cache entry header.");

    std::filesystem::remove_all(directory);
  }

  return 0;
}
