/**
 *
 *  @file cache_entry.cpp
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
#include <cgride/cache/cache_entry.hpp>

#include <system_error>
#include <utility>

namespace cgride::cache
{
  CacheEntry::CacheEntry(CacheKey key)
      : key_(std::move(key))
  {
  }

  CacheEntry::CacheEntry(
      CacheKey key,
      std::vector<std::filesystem::path> outputs)
      : key_(std::move(key)),
        outputs_(std::move(outputs))
  {
  }

  CacheEntry CacheEntry::create(CacheKey key)
  {
    return CacheEntry(std::move(key));
  }

  CacheEntry &CacheEntry::key(CacheKey key)
  {
    key_ = std::move(key);
    return *this;
  }

  CacheEntry &CacheEntry::input(FileSignature signature)
  {
    inputs_.push_back(std::move(signature));
    return *this;
  }

  CacheEntry &CacheEntry::inputs(std::vector<FileSignature> signatures)
  {
    for (auto &signature : signatures)
    {
      inputs_.push_back(std::move(signature));
    }

    return *this;
  }

  CacheEntry &CacheEntry::output(std::filesystem::path path)
  {
    outputs_.push_back(std::move(path));
    return *this;
  }

  CacheEntry &CacheEntry::outputs(std::vector<std::filesystem::path> paths)
  {
    for (auto &path : paths)
    {
      outputs_.push_back(std::move(path));
    }

    return *this;
  }

  CacheEntry &CacheEntry::producer(std::string producer)
  {
    producer_ = std::move(producer);
    return *this;
  }

  CacheEntry &CacheEntry::created_at(TimePoint time) noexcept
  {
    created_at_ = time;
    return *this;
  }

  CacheEntry &CacheEntry::last_used_at(TimePoint time) noexcept
  {
    last_used_at_ = time;
    return *this;
  }

  CacheEntry &CacheEntry::mark_used()
  {
    last_used_at_ = Clock::now();
    ++hit_count_;

    return *this;
  }

  CacheEntry &CacheEntry::hit_count(std::size_t count) noexcept
  {
    hit_count_ = count;
    return *this;
  }

  const CacheKey &CacheEntry::key() const noexcept
  {
    return key_;
  }

  const std::vector<FileSignature> &CacheEntry::inputs() const noexcept
  {
    return inputs_;
  }

  const std::vector<std::filesystem::path> &CacheEntry::outputs() const noexcept
  {
    return outputs_;
  }

  const std::string &CacheEntry::producer() const noexcept
  {
    return producer_;
  }

  CacheEntry::TimePoint CacheEntry::created_at() const noexcept
  {
    return created_at_;
  }

  const std::optional<CacheEntry::TimePoint> &CacheEntry::last_used_at() const noexcept
  {
    return last_used_at_;
  }

  bool CacheEntry::has_last_used_at() const noexcept
  {
    return last_used_at_.has_value();
  }

  std::size_t CacheEntry::hit_count() const noexcept
  {
    return hit_count_;
  }

  bool CacheEntry::valid() const noexcept
  {
    return key_.valid();
  }

  bool CacheEntry::outputs_exist() const
  {
    if (outputs_.empty())
    {
      return false;
    }

    for (const auto &output : outputs_)
    {
      if (output.empty())
      {
        return false;
      }

      std::error_code error;

      if (!std::filesystem::exists(output, error) || error)
      {
        return false;
      }

      if (!std::filesystem::is_regular_file(output, error) || error)
      {
        return false;
      }
    }

    return true;
  }

  bool CacheEntry::inputs_match(bool hash_contents) const
  {
    for (const auto &signature : inputs_)
    {
      if (!signature.valid())
      {
        return false;
      }

      auto current = FileSignature::from_file(signature.path(), hash_contents);

      if (!current)
      {
        return false;
      }

      if (!signature.matches(current.value()))
      {
        return false;
      }
    }

    return true;
  }

} // namespace cgride::cache
