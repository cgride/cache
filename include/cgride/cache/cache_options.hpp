/**
 *
 *  @file cache_options.hpp
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
#ifndef CGRIDE_CACHE_CACHE_OPTIONS_HPP
#define CGRIDE_CACHE_CACHE_OPTIONS_HPP

#include <filesystem>

namespace cgride::cache
{
  /**
   * @class CacheOptions
   * @brief Options controlling local cache behavior.
   *
   * CacheOptions describes where cache data is stored and how aggressively the
   * cache should verify entries. It does not read or write cache entries by
   * itself.
   */
  class CacheOptions
  {
  public:
    /**
     * @brief Construct default cache options.
     */
    CacheOptions() = default;

    /**
     * @brief Construct cache options with a root directory.
     *
     * @param root Cache root directory.
     */
    explicit CacheOptions(std::filesystem::path root);

    /**
     * @brief Create default cache options.
     *
     * @return Cache options.
     */
    [[nodiscard]] static CacheOptions defaults();

    /**
     * @brief Create disabled cache options.
     *
     * @return Cache options with cache disabled.
     */
    [[nodiscard]] static CacheOptions disabled();

    /**
     * @brief Set the cache root directory.
     *
     * @param root Cache root directory.
     * @return Reference to these options.
     */
    CacheOptions &root(std::filesystem::path root);

    /**
     * @brief Enable or disable cache usage.
     *
     * @param value True to enable the cache.
     * @return Reference to these options.
     */
    CacheOptions &enabled(bool value) noexcept;

    /**
     * @brief Control whether output files are verified before a hit is accepted.
     *
     * @param value True to verify output files.
     * @return Reference to these options.
     */
    CacheOptions &verify_outputs(bool value) noexcept;

    /**
     * @brief Control whether input file contents are hashed.
     *
     * @param value True to hash input file contents.
     * @return Reference to these options.
     */
    CacheOptions &hash_inputs(bool value) noexcept;

    /**
     * @brief Control whether missing cache directories are created.
     *
     * @param value True to create cache directories when needed.
     * @return Reference to these options.
     */
    CacheOptions &create_directories(bool value) noexcept;

    /**
     * @brief Access the cache root directory.
     */
    [[nodiscard]] const std::filesystem::path &root() const noexcept;

    /**
     * @brief Access the metadata entries directory.
     */
    [[nodiscard]] std::filesystem::path entries_directory() const;

    /**
     * @brief Access the cached objects directory.
     */
    [[nodiscard]] std::filesystem::path objects_directory() const;

    /**
     * @brief Return true when the cache is enabled.
     */
    [[nodiscard]] bool enabled() const noexcept;

    /**
     * @brief Return true when outputs should be verified before cache hits.
     */
    [[nodiscard]] bool verify_outputs() const noexcept;

    /**
     * @brief Return true when input file contents should be hashed.
     */
    [[nodiscard]] bool hash_inputs() const noexcept;

    /**
     * @brief Return true when cache directories should be created if missing.
     */
    [[nodiscard]] bool create_directories() const noexcept;

    /**
     * @brief Return true when options are usable.
     */
    [[nodiscard]] bool valid() const noexcept;

  private:
    std::filesystem::path root_{".cgride/cache"};
    bool enabled_{true};
    bool verify_outputs_{true};
    bool hash_inputs_{false};
    bool create_directories_{true};
  };

} // namespace cgride::cache

#endif // CGRIDE_CACHE_CACHE_OPTIONS_HPP
