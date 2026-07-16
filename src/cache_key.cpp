/**
 *
 *  @file cache_key.cpp
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
#include <cgride/cache/cache_key.hpp>

#include <functional>
#include <utility>

namespace cgride::cache
{
  namespace
  {
    [[nodiscard]] bool is_filesystem_safe_character(char character) noexcept
    {
      return (character >= 'a' && character <= 'z') ||
             (character >= 'A' && character <= 'Z') ||
             (character >= '0' && character <= '9') ||
             character == '-' ||
             character == '_' ||
             character == '.';
    }

    [[nodiscard]] std::string escape_part(std::string_view part)
    {
      std::string escaped;
      escaped.reserve(part.size());

      for (const auto character : part)
      {
        if (character == '|')
        {
          escaped += "%7C";
          continue;
        }

        if (character == '%')
        {
          escaped += "%25";
          continue;
        }

        escaped.push_back(character);
      }

      return escaped;
    }

  } // namespace

  CacheKey::CacheKey(std::string value)
      : value_(std::move(value))
  {
  }

  CacheKey CacheKey::from_string(std::string value)
  {
    return CacheKey(std::move(value));
  }

  CacheKey CacheKey::from_parts(std::vector<std::string> parts)
  {
    std::string value;

    for (std::size_t index = 0; index < parts.size(); ++index)
    {
      if (index > 0)
      {
        value.push_back('|');
      }

      value += escape_part(parts[index]);
    }

    return CacheKey(std::move(value));
  }

  const std::string &CacheKey::value() const noexcept
  {
    return value_;
  }

  bool CacheKey::empty() const noexcept
  {
    return value_.empty();
  }

  bool CacheKey::valid() const noexcept
  {
    return !value_.empty();
  }

  std::string CacheKey::filesystem_name() const
  {
    std::string output;
    output.reserve(value_.size());

    for (const auto character : value_)
    {
      if (is_filesystem_safe_character(character))
      {
        output.push_back(character);
      }
      else
      {
        output.push_back('_');
      }
    }

    return output;
  }

  bool operator==(const CacheKey &lhs, const CacheKey &rhs) noexcept
  {
    return lhs.value_ == rhs.value_;
  }

  bool operator!=(const CacheKey &lhs, const CacheKey &rhs) noexcept
  {
    return !(lhs == rhs);
  }

  bool operator<(const CacheKey &lhs, const CacheKey &rhs) noexcept
  {
    return lhs.value_ < rhs.value_;
  }

  std::string_view to_string(const CacheKey &key) noexcept
  {
    return key.value();
  }

  std::size_t CacheKeyHash::operator()(const CacheKey &key) const noexcept
  {
    return std::hash<std::string>{}(key.value());
  }

} // namespace cgride::cache
