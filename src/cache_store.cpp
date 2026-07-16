/**
 *
 *  @file cache_store.cpp
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
#include <cgride/cache/cache_store.hpp>

#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>

#include <cgride/core/error.hpp>

namespace cgride::cache
{
  namespace
  {
    using cgride::core::Error;
    using cgride::core::ErrorCode;

    constexpr const char *entry_header = "cgride-cache-entry-v1";

    [[nodiscard]] std::int64_t system_time_to_milliseconds(
        CacheEntry::TimePoint time) noexcept
    {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
                 time.time_since_epoch())
          .count();
    }

    [[nodiscard]] CacheEntry::TimePoint milliseconds_to_system_time(
        std::int64_t value) noexcept
    {
      return CacheEntry::TimePoint(std::chrono::milliseconds(value));
    }

    [[nodiscard]] std::filesystem::file_time_type file_time_from_count(
        std::filesystem::file_time_type::duration::rep value) noexcept
    {
      return std::filesystem::file_time_type(
          std::filesystem::file_time_type::duration(value));
    }

    [[nodiscard]] std::filesystem::file_time_type::duration::rep file_time_count(
        std::filesystem::file_time_type time) noexcept
    {
      return time.time_since_epoch().count();
    }

    [[nodiscard]] std::string bool_to_string(bool value)
    {
      return value ? "true" : "false";
    }

    [[nodiscard]] bool string_to_bool(const std::string &value)
    {
      return value == "true" || value == "1";
    }

    [[nodiscard]] std::string value_after_prefix(
        const std::string &line,
        const std::string &prefix)
    {
      if (!line.starts_with(prefix))
      {
        return {};
      }

      return line.substr(prefix.size());
    }

    [[nodiscard]] cgride::core::Result<std::size_t> parse_size(
        const std::string &value,
        const std::string &field)
    {
      try
      {
        return static_cast<std::size_t>(std::stoull(value));
      }
      catch (...)
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Invalid numeric cache entry field.",
            field);
      }
    }

    [[nodiscard]] cgride::core::Result<std::uintmax_t> parse_uintmax(
        const std::string &value,
        const std::string &field)
    {
      try
      {
        return static_cast<std::uintmax_t>(std::stoull(value));
      }
      catch (...)
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Invalid numeric cache entry field.",
            field);
      }
    }

    [[nodiscard]] cgride::core::Result<std::int64_t> parse_int64(
        const std::string &value,
        const std::string &field)
    {
      try
      {
        return static_cast<std::int64_t>(std::stoll(value));
      }
      catch (...)
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Invalid numeric cache entry field.",
            field);
      }
    }

    [[nodiscard]] cgride::core::Result<std::filesystem::file_time_type::duration::rep>
    parse_file_time_count(const std::string &value, const std::string &field)
    {
      try
      {
        return static_cast<std::filesystem::file_time_type::duration::rep>(
            std::stoll(value));
      }
      catch (...)
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Invalid numeric cache entry field.",
            field);
      }
    }

    [[nodiscard]] cgride::core::Result<void> ensure_directories(
        const CacheOptions &options)
    {
      if (!options.enabled())
      {
        return cgride::core::Result<void>::ok();
      }

      if (!options.valid())
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cannot initialize cache with invalid options.");
      }

      if (!options.create_directories())
      {
        return cgride::core::Result<void>::ok();
      }

      std::error_code error;

      std::filesystem::create_directories(options.entries_directory(), error);

      if (error)
      {
        return Error(
            ErrorCode::IoError,
            "Failed to create cache entries directory.",
            error.message());
      }

      std::filesystem::create_directories(options.objects_directory(), error);

      if (error)
      {
        return Error(
            ErrorCode::IoError,
            "Failed to create cache objects directory.",
            error.message());
      }

      return cgride::core::Result<void>::ok();
    }

    void write_entry(std::ostream &stream, const CacheEntry &entry)
    {
      stream << entry_header << '\n';

      stream << "key=" << entry.key().value() << '\n';
      stream << "producer=" << entry.producer() << '\n';
      stream << "created_at_ms=" << system_time_to_milliseconds(entry.created_at()) << '\n';
      stream << "has_last_used_at=" << bool_to_string(entry.has_last_used_at()) << '\n';

      if (entry.has_last_used_at())
      {
        stream << "last_used_at_ms="
               << system_time_to_milliseconds(entry.last_used_at().value())
               << '\n';
      }
      else
      {
        stream << "last_used_at_ms=0\n";
      }

      stream << "hit_count=" << entry.hit_count() << '\n';

      stream << "input_count=" << entry.inputs().size() << '\n';

      for (std::size_t index = 0; index < entry.inputs().size(); ++index)
      {
        const auto &input = entry.inputs()[index];

        stream << "input." << index << ".path=" << input.path().string() << '\n';
        stream << "input." << index << ".has_size=" << bool_to_string(input.has_size()) << '\n';
        stream << "input." << index << ".size="
               << (input.has_size() ? input.size().value() : 0)
               << '\n';
        stream << "input." << index << ".has_last_write_time="
               << bool_to_string(input.has_last_write_time())
               << '\n';
        stream << "input." << index << ".last_write_time="
               << (input.has_last_write_time()
                       ? file_time_count(input.last_write_time().value())
                       : 0)
               << '\n';
        stream << "input." << index << ".has_content_hash="
               << bool_to_string(input.has_content_hash())
               << '\n';
        stream << "input." << index << ".content_hash="
               << (input.has_content_hash() ? input.content_hash().value() : "")
               << '\n';
      }

      stream << "output_count=" << entry.outputs().size() << '\n';

      for (std::size_t index = 0; index < entry.outputs().size(); ++index)
      {
        stream << "output." << index << ".path="
               << entry.outputs()[index].string()
               << '\n';
      }
    }

    [[nodiscard]] cgride::core::Result<CacheEntry> read_entry(std::istream &stream)
    {
      std::string line;

      if (!std::getline(stream, line) || line != entry_header)
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Invalid cache entry header.");
      }

      if (!std::getline(stream, line) || !line.starts_with("key="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing key.");
      }

      CacheEntry entry(
          CacheKey::from_string(value_after_prefix(line, "key=")));

      if (!std::getline(stream, line) || !line.starts_with("producer="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing producer.");
      }

      entry.producer(value_after_prefix(line, "producer="));

      if (!std::getline(stream, line) || !line.starts_with("created_at_ms="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing creation time.");
      }

      auto created_at = parse_int64(
          value_after_prefix(line, "created_at_ms="),
          "created_at_ms");

      if (!created_at)
      {
        return created_at.error();
      }

      entry.created_at(milliseconds_to_system_time(created_at.value()));

      if (!std::getline(stream, line) || !line.starts_with("has_last_used_at="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing last used marker.");
      }

      const auto has_last_used_at = string_to_bool(
          value_after_prefix(line, "has_last_used_at="));

      if (!std::getline(stream, line) || !line.starts_with("last_used_at_ms="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing last used time.");
      }

      auto last_used_at = parse_int64(
          value_after_prefix(line, "last_used_at_ms="),
          "last_used_at_ms");

      if (!last_used_at)
      {
        return last_used_at.error();
      }

      if (has_last_used_at)
      {
        entry.last_used_at(milliseconds_to_system_time(last_used_at.value()));
      }

      if (!std::getline(stream, line) || !line.starts_with("hit_count="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing hit count.");
      }

      auto hit_count = parse_size(
          value_after_prefix(line, "hit_count="),
          "hit_count");

      if (!hit_count)
      {
        return hit_count.error();
      }

      entry.hit_count(hit_count.value());

      if (!std::getline(stream, line) || !line.starts_with("input_count="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing input count.");
      }

      auto input_count = parse_size(
          value_after_prefix(line, "input_count="),
          "input_count");

      if (!input_count)
      {
        return input_count.error();
      }

      for (std::size_t index = 0; index < input_count.value(); ++index)
      {
        const auto prefix = "input." + std::to_string(index) + ".";

        if (!std::getline(stream, line) || !line.starts_with(prefix + "path="))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing input path.",
              std::to_string(index));
        }

        FileSignature signature(
            std::filesystem::path(value_after_prefix(line, prefix + "path=")));

        if (!std::getline(stream, line) || !line.starts_with(prefix + "has_size="))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing input size marker.",
              std::to_string(index));
        }

        const auto has_size = string_to_bool(value_after_prefix(line, prefix + "has_size="));

        if (!std::getline(stream, line) || !line.starts_with(prefix + "size="))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing input size.",
              std::to_string(index));
        }

        auto size = parse_uintmax(value_after_prefix(line, prefix + "size="), prefix + "size");

        if (!size)
        {
          return size.error();
        }

        if (has_size)
        {
          signature.size(size.value());
        }

        if (!std::getline(stream, line) || !line.starts_with(prefix + "has_last_write_time="))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing input time marker.",
              std::to_string(index));
        }

        const auto has_last_write_time =
            string_to_bool(value_after_prefix(line, prefix + "has_last_write_time="));

        if (!std::getline(stream, line) || !line.starts_with(prefix + "last_write_time="))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing input time.",
              std::to_string(index));
        }

        auto last_write_time = parse_file_time_count(
            value_after_prefix(line, prefix + "last_write_time="),
            prefix + "last_write_time");

        if (!last_write_time)
        {
          return last_write_time.error();
        }

        if (has_last_write_time)
        {
          signature.last_write_time(file_time_from_count(last_write_time.value()));
        }

        if (!std::getline(stream, line) || !line.starts_with(prefix + "has_content_hash="))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing input hash marker.",
              std::to_string(index));
        }

        const auto has_content_hash =
            string_to_bool(value_after_prefix(line, prefix + "has_content_hash="));

        if (!std::getline(stream, line) || !line.starts_with(prefix + "content_hash="))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing input hash.",
              std::to_string(index));
        }

        if (has_content_hash)
        {
          signature.content_hash(value_after_prefix(line, prefix + "content_hash="));
        }

        entry.input(std::move(signature));
      }

      if (!std::getline(stream, line) || !line.starts_with("output_count="))
      {
        return Error(
            ErrorCode::InvalidArgument,
            "Cache entry is missing output count.");
      }

      auto output_count = parse_size(
          value_after_prefix(line, "output_count="),
          "output_count");

      if (!output_count)
      {
        return output_count.error();
      }

      for (std::size_t index = 0; index < output_count.value(); ++index)
      {
        const auto prefix = "output." + std::to_string(index) + ".path=";

        if (!std::getline(stream, line) || !line.starts_with(prefix))
        {
          return Error(
              ErrorCode::InvalidArgument,
              "Cache entry is missing output path.",
              std::to_string(index));
        }

        entry.output(std::filesystem::path(value_after_prefix(line, prefix)));
      }

      return entry;
    }

  } // namespace

  CacheStore::CacheStore(CacheOptions options)
      : options_(std::move(options))
  {
  }

  const CacheOptions &CacheStore::options() const noexcept
  {
    return options_;
  }

  CacheStore &CacheStore::options(CacheOptions options)
  {
    options_ = std::move(options);
    return *this;
  }

  cgride::core::Result<void> CacheStore::initialize() const
  {
    return ensure_directories(options_);
  }

  std::filesystem::path CacheStore::entry_path(const CacheKey &key) const
  {
    return options_.entries_directory() / (key.filesystem_name() + ".entry");
  }

  std::filesystem::path CacheStore::object_directory(const CacheKey &key) const
  {
    return options_.objects_directory() / key.filesystem_name();
  }

  bool CacheStore::contains(const CacheKey &key) const
  {
    if (!options_.enabled() || !key.valid())
    {
      return false;
    }

    std::error_code error;

    return std::filesystem::exists(entry_path(key), error) &&
           !error &&
           std::filesystem::is_regular_file(entry_path(key), error) &&
           !error;
  }

  cgride::core::Result<void> CacheStore::put(const CacheEntry &entry) const
  {
    if (!options_.enabled())
    {
      return cgride::core::Result<void>::ok();
    }

    if (!options_.valid())
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Cannot store cache entry with invalid cache options.");
    }

    if (!entry.valid())
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Cannot store an invalid cache entry.");
    }

    auto initialized = initialize();

    if (!initialized)
    {
      return initialized.error();
    }

    std::ofstream file(entry_path(entry.key()), std::ios::binary | std::ios::trunc);

    if (!file)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to open cache entry for writing.",
          entry_path(entry.key()));
    }

    write_entry(file, entry);

    if (!file)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to write cache entry.",
          entry_path(entry.key()));
    }

    return cgride::core::Result<void>::ok();
  }

  cgride::core::Result<CacheEntry> CacheStore::get(const CacheKey &key) const
  {
    if (!options_.enabled())
    {
      return Error(
          ErrorCode::InvalidState,
          "Cannot read cache entry while cache is disabled.");
    }

    if (!key.valid())
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Cannot read cache entry with an invalid key.");
    }

    const auto path = entry_path(key);

    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
      return Error(
          ErrorCode::NotFound,
          "Cache entry was not found.",
          path);
    }

    auto entry = read_entry(file);

    if (!entry)
    {
      return entry.error();
    }

    if (entry.value().key() != key)
    {
      return Error(
          ErrorCode::InvalidState,
          "Cache entry key does not match requested key.",
          key.value());
    }

    return entry.value();
  }

  cgride::core::Result<void> CacheStore::remove(const CacheKey &key) const
  {
    if (!options_.enabled())
    {
      return cgride::core::Result<void>::ok();
    }

    if (!key.valid())
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Cannot remove cache entry with an invalid key.");
    }

    std::error_code error;

    std::filesystem::remove(entry_path(key), error);

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to remove cache entry.",
          error.message());
    }

    std::filesystem::remove_all(object_directory(key), error);

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to remove cache object directory.",
          error.message());
    }

    return cgride::core::Result<void>::ok();
  }

  cgride::core::Result<void> CacheStore::clear() const
  {
    if (!options_.enabled())
    {
      return cgride::core::Result<void>::ok();
    }

    std::error_code error;

    std::filesystem::remove_all(options_.root(), error);

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to clear cache root.",
          error.message());
    }

    return initialize();
  }

  cgride::core::Result<std::vector<CacheKey>> CacheStore::keys() const
  {
    std::vector<CacheKey> keys;

    if (!options_.enabled())
    {
      return keys;
    }

    std::error_code error;

    if (!std::filesystem::exists(options_.entries_directory(), error))
    {
      return keys;
    }

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to inspect cache entries directory.",
          error.message());
    }

    for (const auto &entry : std::filesystem::directory_iterator(options_.entries_directory(), error))
    {
      if (error)
      {
        return Error(
            ErrorCode::IoError,
            "Failed to iterate cache entries directory.",
            error.message());
      }

      if (!entry.is_regular_file(error) || error)
      {
        continue;
      }

      if (entry.path().extension() != ".entry")
      {
        continue;
      }

      std::ifstream file(entry.path(), std::ios::binary);

      if (!file)
      {
        continue;
      }

      auto parsed = read_entry(file);

      if (parsed && parsed.value().key().valid())
      {
        keys.push_back(parsed.value().key());
      }
    }

    return keys;
  }

} // namespace cgride::cache
