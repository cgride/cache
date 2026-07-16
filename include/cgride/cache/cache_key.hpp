/**
 *
 *  @file cache_key.hpp
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
#ifndef CGRIDE_CACHE_CACHE_KEY_HPP
#define CGRIDE_CACHE_CACHE_KEY_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace cgride::cache
{
  /**
   * @class CacheKey
   * @brief Stable identifier for one cache entry.
   *
   * CacheKey is a string-backed key produced by higher-level modules from
   * inputs such as command data, compiler identity, source signatures and
   * build options. The cache module stores and compares keys, but it does not
   * decide which project target or graph task created them.
   */
  class CacheKey
  {
  public:
    /**
     * @brief Construct an empty cache key.
     */
    CacheKey() = default;

    /**
     * @brief Construct a cache key from a string.
     *
     * @param value Cache key value.
     */
    explicit CacheKey(std::string value);

    /**
     * @brief Create a cache key from a string.
     *
     * @param value Cache key value.
     * @return Cache key.
     */
    [[nodiscard]] static CacheKey from_string(std::string value);

    /**
     * @brief Create a cache key by joining stable string parts.
     *
     * This helper is deterministic for the provided parts. It does not hash
     * the parts; it creates a readable namespaced key string.
     *
     * @param parts Stable key parts.
     * @return Cache key.
     */
    [[nodiscard]] static CacheKey from_parts(std::vector<std::string> parts);

    /**
     * @brief Access the cache key value.
     */
    [[nodiscard]] const std::string &value() const noexcept;

    /**
     * @brief Return true when the key is empty.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Return true when the key is usable.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return a filesystem-safe representation of the key.
     *
     * Unsafe characters are replaced with an underscore.
     *
     * @return Filesystem-safe key string.
     */
    [[nodiscard]] std::string filesystem_name() const;

    /**
     * @brief Compare two cache keys.
     */
    [[nodiscard]] friend bool operator==(const CacheKey &lhs,
                                         const CacheKey &rhs) noexcept;

    /**
     * @brief Compare two cache keys.
     */
    [[nodiscard]] friend bool operator!=(const CacheKey &lhs,
                                         const CacheKey &rhs) noexcept;

    /**
     * @brief Order cache keys by value.
     */
    [[nodiscard]] friend bool operator<(const CacheKey &lhs,
                                        const CacheKey &rhs) noexcept;

  private:
    std::string value_{};
  };

  /**
   * @brief Return the string value of a cache key.
   *
   * @param key Cache key.
   * @return Cache key string.
   */
  [[nodiscard]] std::string_view to_string(const CacheKey &key) noexcept;

  /**
   * @struct CacheKeyHash
   * @brief Hash functor for CacheKey.
   *
   * This hash is intended for in-memory containers only. It should not be used
   * as the persistent cache key stored on disk.
   */
  struct CacheKeyHash
  {
    /**
     * @brief Hash a cache key.
     *
     * @param key Cache key.
     * @return Hash value.
     */
    [[nodiscard]] std::size_t operator()(const CacheKey &key) const noexcept;
  };

} // namespace cgride::cache

#endif // CGRIDE_CACHE_CACHE_KEY_HPP
