/**
 *
 *  @file cache_entry.hpp
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
#ifndef CGRIDE_CACHE_CACHE_ENTRY_HPP
#define CGRIDE_CACHE_CACHE_ENTRY_HPP

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <cgride/cache/cache_key.hpp>
#include <cgride/cache/file_signature.hpp>

namespace cgride::cache
{
  /**
   * @class CacheEntry
   * @brief Metadata recorded for one cacheable operation.
   *
   * CacheEntry stores the stable key, observed input signatures and output
   * paths for one cached operation. It does not decide which build target,
   * graph task, compiler or executor produced the entry.
   */
  class CacheEntry
  {
  public:
    /**
     * @brief Clock used for cache metadata timestamps.
     */
    using Clock = std::chrono::system_clock;

    /**
     * @brief Time point used for cache metadata timestamps.
     */
    using TimePoint = Clock::time_point;

    /**
     * @brief Construct an empty cache entry.
     */
    CacheEntry() = default;

    /**
     * @brief Construct a cache entry from a key.
     *
     * @param key Cache key.
     */
    explicit CacheEntry(CacheKey key);

    /**
     * @brief Construct a cache entry from a key and output paths.
     *
     * @param key Cache key.
     * @param outputs Output paths.
     */
    CacheEntry(CacheKey key, std::vector<std::filesystem::path> outputs);

    /**
     * @brief Create a cache entry from a key.
     *
     * @param key Cache key.
     * @return Cache entry.
     */
    [[nodiscard]] static CacheEntry create(CacheKey key);

    /**
     * @brief Set the cache key.
     *
     * @param key Cache key.
     * @return Reference to this entry.
     */
    CacheEntry &key(CacheKey key);

    /**
     * @brief Add one input file signature.
     *
     * @param signature Input file signature.
     * @return Reference to this entry.
     */
    CacheEntry &input(FileSignature signature);

    /**
     * @brief Add many input file signatures.
     *
     * @param signatures Input file signatures.
     * @return Reference to this entry.
     */
    CacheEntry &inputs(std::vector<FileSignature> signatures);

    /**
     * @brief Add one output path.
     *
     * @param path Output path.
     * @return Reference to this entry.
     */
    CacheEntry &output(std::filesystem::path path);

    /**
     * @brief Add many output paths.
     *
     * @param paths Output paths.
     * @return Reference to this entry.
     */
    CacheEntry &outputs(std::vector<std::filesystem::path> paths);

    /**
     * @brief Set a human-readable producer name.
     *
     * This can be a task id, command kind or other diagnostic label.
     *
     * @param producer Producer label.
     * @return Reference to this entry.
     */
    CacheEntry &producer(std::string producer);

    /**
     * @brief Set the creation time.
     *
     * @param time Creation time.
     * @return Reference to this entry.
     */
    CacheEntry &created_at(TimePoint time) noexcept;

    /**
     * @brief Set the last used time.
     *
     * @param time Last used time.
     * @return Reference to this entry.
     */
    CacheEntry &last_used_at(TimePoint time) noexcept;

    /**
     * @brief Mark the entry as used now and increment the hit count.
     *
     * @return Reference to this entry.
     */
    CacheEntry &mark_used();

    /**
     * @brief Set the hit count.
     *
     * @param count Hit count.
     * @return Reference to this entry.
     */
    CacheEntry &hit_count(std::size_t count) noexcept;

    /**
     * @brief Access the cache key.
     */
    [[nodiscard]] const CacheKey &key() const noexcept;

    /**
     * @brief Access input file signatures.
     */
    [[nodiscard]] const std::vector<FileSignature> &inputs() const noexcept;

    /**
     * @brief Access output paths.
     */
    [[nodiscard]] const std::vector<std::filesystem::path> &outputs() const noexcept;

    /**
     * @brief Access the optional producer label.
     */
    [[nodiscard]] const std::string &producer() const noexcept;

    /**
     * @brief Access the creation time.
     */
    [[nodiscard]] TimePoint created_at() const noexcept;

    /**
     * @brief Access the optional last used time.
     */
    [[nodiscard]] const std::optional<TimePoint> &last_used_at() const noexcept;

    /**
     * @brief Return true when the entry has a last used time.
     */
    [[nodiscard]] bool has_last_used_at() const noexcept;

    /**
     * @brief Access the hit count.
     */
    [[nodiscard]] std::size_t hit_count() const noexcept;

    /**
     * @brief Return true when the entry has a valid key.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true when all output paths exist as regular files.
     *
     * @return True when every output path exists.
     */
    [[nodiscard]] bool outputs_exist() const;

    /**
     * @brief Return true when recorded input signatures still match the files.
     *
     * If hash_contents is true, current signatures include content hashes.
     *
     * @param hash_contents Whether to hash files before comparing.
     * @return True when all inputs still match.
     */
    [[nodiscard]] bool inputs_match(bool hash_contents = false) const;

  private:
    CacheKey key_{};
    std::vector<FileSignature> inputs_{};
    std::vector<std::filesystem::path> outputs_{};
    std::string producer_{};
    TimePoint created_at_{Clock::now()};
    std::optional<TimePoint> last_used_at_{};
    std::size_t hit_count_{0};
  };

} // namespace cgride::cache

#endif // CGRIDE_CACHE_CACHE_ENTRY_HPP
