/**
 *
 *  @file file_signature.cpp
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
#include <cgride/cache/file_signature.hpp>

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>
#include <utility>

#include <cgride/core/error.hpp>

namespace cgride::cache
{
  namespace
  {
    using cgride::core::Error;
    using cgride::core::ErrorCode;

    constexpr std::uint64_t fnv_offset_basis = 14695981039346656037ull;
    constexpr std::uint64_t fnv_prime = 1099511628211ull;

    [[nodiscard]] std::string to_hex(std::uint64_t value)
    {
      std::ostringstream stream;

      stream << std::hex
             << std::setfill('0')
             << std::setw(16)
             << value;

      return stream.str();
    }

  } // namespace

  FileSignature::FileSignature(std::filesystem::path path)
      : path_(std::move(path))
  {
  }

  cgride::core::Result<FileSignature> FileSignature::from_file(
      std::filesystem::path path,
      bool hash_contents)
  {
    if (path.empty())
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Cannot create file signature from an empty path.");
    }

    std::error_code error;

    if (!std::filesystem::exists(path, error))
    {
      return Error(
          ErrorCode::NotFound,
          "File does not exist.",
          path);
    }

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to inspect file existence.",
          error.message());
    }

    if (!std::filesystem::is_regular_file(path, error))
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Path is not a regular file.",
          path);
    }

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to inspect file type.",
          error.message());
    }

    auto file_size = std::filesystem::file_size(path, error);

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to read file size.",
          error.message());
    }

    auto write_time = std::filesystem::last_write_time(path, error);

    if (error)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to read file last write time.",
          error.message());
    }

    FileSignature signature(std::move(path));

    signature
        .size(file_size)
        .last_write_time(write_time);

    if (hash_contents)
    {
      auto hash = hash_file_contents(signature.path());

      if (!hash)
      {
        return hash.error();
      }

      signature.content_hash(hash.value());
    }

    return signature;
  }

  FileSignature &FileSignature::path(std::filesystem::path path)
  {
    path_ = std::move(path);
    return *this;
  }

  FileSignature &FileSignature::size(std::uintmax_t size) noexcept
  {
    size_ = size;
    return *this;
  }

  FileSignature &FileSignature::last_write_time(
      std::filesystem::file_time_type time) noexcept
  {
    last_write_time_ = time;
    return *this;
  }

  FileSignature &FileSignature::content_hash(std::string hash)
  {
    content_hash_ = std::move(hash);
    return *this;
  }

  FileSignature &FileSignature::clear_content_hash() noexcept
  {
    content_hash_.reset();
    return *this;
  }

  const std::filesystem::path &FileSignature::path() const noexcept
  {
    return path_;
  }

  const std::optional<std::uintmax_t> &FileSignature::size() const noexcept
  {
    return size_;
  }

  bool FileSignature::has_size() const noexcept
  {
    return size_.has_value();
  }

  const std::optional<std::filesystem::file_time_type> &
  FileSignature::last_write_time() const noexcept
  {
    return last_write_time_;
  }

  bool FileSignature::has_last_write_time() const noexcept
  {
    return last_write_time_.has_value();
  }

  const std::optional<std::string> &FileSignature::content_hash() const noexcept
  {
    return content_hash_;
  }

  bool FileSignature::has_content_hash() const noexcept
  {
    return content_hash_.has_value();
  }

  bool FileSignature::valid() const noexcept
  {
    return !path_.empty() &&
           size_.has_value() &&
           last_write_time_.has_value();
  }

  bool FileSignature::matches(const FileSignature &other) const noexcept
  {
    if (!valid() || !other.valid())
    {
      return false;
    }

    if (path_ != other.path_)
    {
      return false;
    }

    if (size_ != other.size_)
    {
      return false;
    }

    if (last_write_time_ != other.last_write_time_)
    {
      return false;
    }

    if (content_hash_.has_value() && other.content_hash_.has_value())
    {
      return content_hash_.value() == other.content_hash_.value();
    }

    return true;
  }

  cgride::core::Result<std::string> hash_file_contents(
      const std::filesystem::path &path)
  {
    if (path.empty())
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Cannot hash file contents from an empty path.");
    }

    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
      return Error(
          ErrorCode::IoError,
          "Failed to open file for hashing.",
          path);
    }

    std::uint64_t hash = fnv_offset_basis;
    std::array<char, 8192> buffer{};

    while (file)
    {
      file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

      const auto count = file.gcount();

      for (std::streamsize index = 0; index < count; ++index)
      {
        hash ^= static_cast<unsigned char>(buffer[static_cast<std::size_t>(index)]);
        hash *= fnv_prime;
      }
    }

    if (file.bad())
    {
      return Error(
          ErrorCode::IoError,
          "Failed while reading file for hashing.",
          path);
    }

    return to_hex(hash);
  }

} // namespace cgride::cache
