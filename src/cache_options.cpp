/**
 *
 *  @file cache_options.cpp
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
#include <cgride/cache/cache_options.hpp>

#include <utility>

namespace cgride::cache
{
  CacheOptions::CacheOptions(std::filesystem::path root)
      : root_(std::move(root))
  {
  }

  CacheOptions CacheOptions::defaults()
  {
    return CacheOptions{};
  }

  CacheOptions CacheOptions::disabled()
  {
    CacheOptions options;

    options.enabled(false);

    return options;
  }

  CacheOptions &CacheOptions::root(std::filesystem::path root)
  {
    root_ = std::move(root);
    return *this;
  }

  CacheOptions &CacheOptions::enabled(bool value) noexcept
  {
    enabled_ = value;
    return *this;
  }

  CacheOptions &CacheOptions::verify_outputs(bool value) noexcept
  {
    verify_outputs_ = value;
    return *this;
  }

  CacheOptions &CacheOptions::hash_inputs(bool value) noexcept
  {
    hash_inputs_ = value;
    return *this;
  }

  CacheOptions &CacheOptions::create_directories(bool value) noexcept
  {
    create_directories_ = value;
    return *this;
  }

  const std::filesystem::path &CacheOptions::root() const noexcept
  {
    return root_;
  }

  std::filesystem::path CacheOptions::entries_directory() const
  {
    return root_ / "entries";
  }

  std::filesystem::path CacheOptions::objects_directory() const
  {
    return root_ / "objects";
  }

  bool CacheOptions::enabled() const noexcept
  {
    return enabled_;
  }

  bool CacheOptions::verify_outputs() const noexcept
  {
    return verify_outputs_;
  }

  bool CacheOptions::hash_inputs() const noexcept
  {
    return hash_inputs_;
  }

  bool CacheOptions::create_directories() const noexcept
  {
    return create_directories_;
  }

  bool CacheOptions::valid() const noexcept
  {
    if (!enabled_)
    {
      return true;
    }

    return !root_.empty();
  }

} // namespace cgride::cache
