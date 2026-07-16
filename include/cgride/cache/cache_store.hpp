/**
 *
 *  @file cache_store.hpp
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
#ifndef CGRIDE_CACHE_CACHE_STORE_HPP
#define CGRIDE_CACHE_CACHE_STORE_HPP

#include <filesystem>
#include <vector>

#include <cgride/cache/cache_entry.hpp>
#include <cgride/cache/cache_key.hpp>
#include <cgride/cache/cache_options.hpp>
#include <cgride/core/result.hpp>

namespace cgride::cache
{
  /**
   * @class CacheStore
   * @brief Local filesystem store for cache entry metadata.
   *
   * CacheStore stores and retrieves cache entry metadata under a local cache
   * root. It does not decide how cache keys are produced, which compiler is
   * used, which graph task owns an entry or whether an executor should skip a
   * task.
   */
  class CacheStore
  {
  public:
    /**
     * @brief Construct a cache store with default options.
     */
    CacheStore() = default;

    /**
     * @brief Construct a cache store with explicit options.
     *
     * @param options Cache options.
     */
    explicit CacheStore(CacheOptions options);

    /**
     * @brief Access cache options.
     */
    [[nodiscard]] const CacheOptions &options() const noexcept;

    /**
     * @brief Set cache options.
     *
     * @param options Cache options.
     * @return Reference to this store.
     */
    CacheStore &options(CacheOptions options);

    /**
     * @brief Initialize cache directories when enabled.
     *
     * @return Success or filesystem error.
     */
    [[nodiscard]] cgride::core::Result<void> initialize() const;

    /**
     * @brief Return the metadata path for a cache key.
     *
     * @param key Cache key.
     * @return Entry metadata path.
     */
    [[nodiscard]] std::filesystem::path entry_path(const CacheKey &key) const;

    /**
     * @brief Return the objects directory for a cache key.
     *
     * @param key Cache key.
     * @return Object directory path.
     */
    [[nodiscard]] std::filesystem::path object_directory(const CacheKey &key) const;

    /**
     * @brief Return true when a valid metadata entry exists for a key.
     *
     * @param key Cache key.
     * @return True when the entry exists.
     */
    [[nodiscard]] bool contains(const CacheKey &key) const;

    /**
     * @brief Store one cache entry metadata file.
     *
     * @param entry Cache entry.
     * @return Success or validation/filesystem error.
     */
    [[nodiscard]] cgride::core::Result<void> put(const CacheEntry &entry) const;

    /**
     * @brief Load one cache entry metadata file.
     *
     * @param key Cache key.
     * @return Cache entry or error.
     */
    [[nodiscard]] cgride::core::Result<CacheEntry> get(const CacheKey &key) const;

    /**
     * @brief Remove one cache entry metadata file and object directory.
     *
     * @param key Cache key.
     * @return Success or filesystem error.
     */
    [[nodiscard]] cgride::core::Result<void> remove(const CacheKey &key) const;

    /**
     * @brief Remove all cache metadata and object files under the cache root.
     *
     * @return Success or filesystem error.
     */
    [[nodiscard]] cgride::core::Result<void> clear() const;

    /**
     * @brief List all cache keys with metadata entries.
     *
     * @return Cache keys or filesystem error.
     */
    [[nodiscard]] cgride::core::Result<std::vector<CacheKey>> keys() const;

  private:
    CacheOptions options_{};
  };

} // namespace cgride::cache

#endif // CGRIDE_CACHE_CACHE_STORE_HPP
