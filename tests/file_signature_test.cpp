/**
 *
 *  @file file_signature_test.cpp
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

#include <cgride/cache/file_signature.hpp>

namespace
{
  [[nodiscard]] std::filesystem::path make_test_directory()
  {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();

    auto directory = std::filesystem::temp_directory_path() /
                     ("cgride_cache_file_signature_test_" + std::to_string(now));

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
    cgride::cache::FileSignature signature;

    assert(signature.path().empty());
    assert(!signature.has_size());
    assert(!signature.size().has_value());
    assert(!signature.has_last_write_time());
    assert(!signature.last_write_time().has_value());
    assert(!signature.has_content_hash());
    assert(!signature.content_hash().has_value());
    assert(!signature.valid());
  }

  {
    cgride::cache::FileSignature signature("src/main.cpp");

    assert(signature.path() == std::filesystem::path("src/main.cpp"));
    assert(!signature.valid());

    auto now = std::filesystem::file_time_type::clock::now();

    signature
        .size(42)
        .last_write_time(now)
        .content_hash("abc123");

    assert(signature.has_size());
    assert(signature.size().value() == 42);
    assert(signature.has_last_write_time());
    assert(signature.last_write_time().value() == now);
    assert(signature.has_content_hash());
    assert(signature.content_hash().value() == "abc123");
    assert(signature.valid());

    signature.clear_content_hash();

    assert(!signature.has_content_hash());
    assert(!signature.content_hash().has_value());
    assert(signature.valid());
  }

  {
    auto result = cgride::cache::FileSignature::from_file({});

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Cannot create file signature from an empty path.");
  }

  {
    auto directory = make_test_directory();
    auto missing = directory / "missing.txt";

    auto result = cgride::cache::FileSignature::from_file(missing);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::NotFound);
    assert(result.error().message() == "File does not exist.");
    assert(result.error().path().has_value());
    assert(result.error().path().value() == missing);

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();

    auto result = cgride::cache::FileSignature::from_file(directory);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Path is not a regular file.");
    assert(result.error().path().has_value());
    assert(result.error().path().value() == directory);

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();
    auto file = directory / "hello.txt";

    write_text_file(file, "hello");

    auto result = cgride::cache::FileSignature::from_file(file);

    assert(result);

    auto signature = result.value();

    assert(signature.path() == file);
    assert(signature.has_size());
    assert(signature.size().value() == 5);
    assert(signature.has_last_write_time());
    assert(!signature.has_content_hash());
    assert(signature.valid());

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();
    auto file = directory / "hash.txt";

    write_text_file(file, "hello");

    auto first = cgride::cache::hash_file_contents(file);
    auto second = cgride::cache::hash_file_contents(file);

    assert(first);
    assert(second);
    assert(!first.value().empty());
    assert(first.value().size() == 16);
    assert(first.value() == second.value());

    write_text_file(file, "hello world");

    auto third = cgride::cache::hash_file_contents(file);

    assert(third);
    assert(third.value().size() == 16);
    assert(third.value() != first.value());

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();
    auto file = directory / "with-hash.txt";

    write_text_file(file, "cache me");

    auto result = cgride::cache::FileSignature::from_file(file, true);

    assert(result);

    auto signature = result.value();

    assert(signature.path() == file);
    assert(signature.has_size());
    assert(signature.size().value() == 8);
    assert(signature.has_last_write_time());
    assert(signature.has_content_hash());
    assert(signature.content_hash().value().size() == 16);
    assert(signature.valid());

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();
    auto file = directory / "match.txt";

    write_text_file(file, "same");

    auto first = cgride::cache::FileSignature::from_file(file, true);
    auto second = cgride::cache::FileSignature::from_file(file, true);

    assert(first);
    assert(second);
    assert(first.value().matches(second.value()));
    assert(second.value().matches(first.value()));

    std::filesystem::remove_all(directory);
  }

  {
    auto directory = make_test_directory();
    auto file = directory / "changed.txt";

    write_text_file(file, "before");

    auto before = cgride::cache::FileSignature::from_file(file, true);

    assert(before);

    std::this_thread::sleep_for(std::chrono::milliseconds{20});

    write_text_file(file, "after-after");

    auto after = cgride::cache::FileSignature::from_file(file, true);

    assert(after);
    assert(!before.value().matches(after.value()));

    std::filesystem::remove_all(directory);
  }

  {
    auto result = cgride::cache::hash_file_contents({});

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Cannot hash file contents from an empty path.");
  }

  return 0;
}
