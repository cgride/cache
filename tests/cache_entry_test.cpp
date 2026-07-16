/**
 *
 *  @file cache_entry_test.cpp
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
#include <thread>
#include <vector>

#include <cgride/cache/cache_entry.hpp>

namespace
{
  [[nodiscard]] std::filesystem::path make_test_directory()
  {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();

    auto directory = std::filesystem::temp_directory_path() /
                     ("cgride_cache_entry_test_" + std::to_string(now));

    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    return directory;
  }

  void write_text_file(const std::filesystem::path &path, const std::string &content)
  {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << content;
  }

} // namespace

int main()
{
  {
    cgride::cache::CacheEntry entry;

    assert(entry.key().empty());
    assert(entry.inputs().empty());
    assert(entry.outputs().empty());
    assert(entry.producer().empty());
    assert(!entry.has_last_used_at());
    assert(!entry.last_used_at().has_value());
    assert(entry.hit_count() == 0);
    assert(!entry.valid());
    assert(!entry.outputs_exist());
    assert(entry.inputs_match());
  }

  {
    auto key = cgride::cache::CacheKey::from_string("compile:main.cpp");

    cgride::cache::CacheEntry entry(key);

    assert(entry.key() == key);
    assert(entry.valid());
  }

  {
    auto key = cgride::cache::CacheKey::from_string("link:app");

    cgride::cache::CacheEntry entry(
        key,
        {
            std::filesystem::path("build/app"),
            std::filesystem::path("build/app.map"),
        });

    assert(entry.key() == key);
    assert(entry.outputs().size() == 2);
    assert(entry.outputs()[0] == std::filesystem::path("build/app"));
    assert(entry.outputs()[1] == std::filesystem::path("build/app.map"));
    assert(entry.valid());
  }

  {
    auto entry = cgride::cache::CacheEntry::create(
        cgride::cache::CacheKey::from_string("archive:core"));

    assert(entry.key() == cgride::cache::CacheKey::from_string("archive:core"));
    assert(entry.valid());
  }

  {
    cgride::cache::CacheEntry entry;

    auto created_at = cgride::cache::CacheEntry::Clock::now();

    entry
        .key(cgride::cache::CacheKey::from_string("compile"))
        .producer("compile:main.cpp")
        .created_at(created_at)
        .hit_count(4);

    assert(entry.key() == cgride::cache::CacheKey::from_string("compile"));
    assert(entry.producer() == "compile:main.cpp");
    assert(entry.created_at() == created_at);
    assert(entry.hit_count() == 4);
    assert(entry.valid());

    entry.mark_used();

    assert(entry.has_last_used_at());
    assert(entry.last_used_at().has_value());
    assert(entry.hit_count() == 5);
  }

  {
    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("outputs"));

    entry
        .output("build/main.o")
        .output("build/app");

    assert(entry.outputs().size() == 2);
    assert(entry.outputs()[0] == std::filesystem::path("build/main.o"));
    assert(entry.outputs()[1] == std::filesystem::path("build/app"));
  }

  {
    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("many-outputs"));

    entry.outputs({
        std::filesystem::path("build/a.o"),
        std::filesystem::path("build/b.o"),
    });

    assert(entry.outputs().size() == 2);
    assert(entry.outputs()[0] == std::filesystem::path("build/a.o"));
    assert(entry.outputs()[1] == std::filesystem::path("build/b.o"));
  }

  {
    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("manual-input"));

    auto now = std::filesystem::file_time_type::clock::now();

    cgride::cache::FileSignature signature("src/main.cpp");

    signature
        .size(123)
        .last_write_time(now)
        .content_hash("abc123");

    entry.input(signature);

    assert(entry.inputs().size() == 1);
    assert(entry.inputs()[0].path() == std::filesystem::path("src/main.cpp"));
    assert(entry.inputs()[0].size().value() == 123);
    assert(entry.inputs()[0].last_write_time().value() == now);
    assert(entry.inputs()[0].content_hash().value() == "abc123");
  }

  {
    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("many-inputs"));

    auto now = std::filesystem::file_time_type::clock::now();

    cgride::cache::FileSignature first("src/a.cpp");
    first.size(10).last_write_time(now);

    cgride::cache::FileSignature second("src/b.cpp");
    second.size(20).last_write_time(now);

    entry.inputs({
        first,
        second,
    });

    assert(entry.inputs().size() == 2);
    assert(entry.inputs()[0].path() == std::filesystem::path("src/a.cpp"));
    assert(entry.inputs()[1].path() == std::filesystem::path("src/b.cpp"));
  }

  {
    auto directory = make_test_directory();

    auto output = directory / "main.o";
    write_text_file(output, "object");

    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("outputs-exist"));

    entry.output(output);

    assert(entry.outputs_exist());

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    auto missing = directory / "missing.o";

    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("missing-output"));

    entry.output(missing);

    assert(!entry.outputs_exist());

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("directory-output"));

    entry.output(directory);

    assert(!entry.outputs_exist());

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    auto input = directory / "main.cpp";
    write_text_file(input, "int main() { return 0; }\n");

    auto signature = cgride::cache::FileSignature::from_file(input, true);

    assert(signature);

    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("input-match"));

    entry.input(signature.value());

    assert(entry.inputs_match(true));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    auto input = directory / "changed.cpp";
    write_text_file(input, "before\n");

    auto signature = cgride::cache::FileSignature::from_file(input, true);

    assert(signature);

    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("input-changed"));

    entry.input(signature.value());

    std::this_thread::sleep_for(std::chrono::milliseconds{20});

    write_text_file(input, "after\n");

    assert(!entry.inputs_match(true));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    auto missing = directory / "missing.cpp";

    auto now = std::filesystem::file_time_type::clock::now();

    cgride::cache::FileSignature signature(missing);
    signature.size(10).last_write_time(now);

    cgride::cache::CacheEntry entry(
        cgride::cache::CacheKey::from_string("missing-input"));

    entry.input(signature);

    assert(!entry.inputs_match());

    std::filesystem::remove_all(directory);
  }

  return 0;
}
