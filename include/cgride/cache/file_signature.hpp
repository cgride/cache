/**
 *
 *  @file file_signature.hpp
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
#ifndef CGRIDE_CACHE_FILE_SIGNATURE_HPP
#define CGRIDE_CACHE_FILE_SIGNATURE_HPP

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include <cgride/core/result.hpp>

namespace cgride::cache
{
  /**
   * @class FileSignature
   * @brief Observable state of one filesystem file.
   *
   * FileSignature stores the file information used by the cache to detect
   * whether a file still matches the state recorded in a cache entry.
   *
   * It does not decide which build task owns the file and it does not restore
   * cached outputs by itself.
   */
  class FileSignature
  {
  public:
    /**
     * @brief Construct an empty file signature.
     */
    FileSignature() = default;

    /**
     * @brief Construct a file signature for a path.
     *
     * @param path File path.
     */
    explicit FileSignature(std::filesystem::path path);

    /**
     * @brief Create a signature for an existing regular file.
     *
     * @param path File path.
     * @param hash_contents Whether to compute a content hash.
     * @return File signature or filesystem error.
     */
    [[nodiscard]] static cgride::core::Result<FileSignature> from_file(
        std::filesystem::path path,
        bool hash_contents = false);

    /**
     * @brief Set the file path.
     *
     * @param path File path.
     * @return Reference to this signature.
     */
    FileSignature &path(std::filesystem::path path);

    /**
     * @brief Set the file size.
     *
     * @param size File size in bytes.
     * @return Reference to this signature.
     */
    FileSignature &size(std::uintmax_t size) noexcept;

    /**
     * @brief Set the file last write time.
     *
     * @param time File last write time.
     * @return Reference to this signature.
     */
    FileSignature &last_write_time(std::filesystem::file_time_type time) noexcept;

    /**
     * @brief Set the optional content hash.
     *
     * @param hash Stable content hash string.
     * @return Reference to this signature.
     */
    FileSignature &content_hash(std::string hash);

    /**
     * @brief Clear the optional content hash.
     *
     * @return Reference to this signature.
     */
    FileSignature &clear_content_hash() noexcept;

    /**
     * @brief Access the file path.
     */
    [[nodiscard]] const std::filesystem::path &path() const noexcept;

    /**
     * @brief Access the optional file size.
     */
    [[nodiscard]] const std::optional<std::uintmax_t> &size() const noexcept;

    /**
     * @brief Return true when a file size is available.
     */
    [[nodiscard]] bool has_size() const noexcept;

    /**
     * @brief Access the optional file last write time.
     */
    [[nodiscard]] const std::optional<std::filesystem::file_time_type> &last_write_time() const noexcept;

    /**
     * @brief Return true when a file last write time is available.
     */
    [[nodiscard]] bool has_last_write_time() const noexcept;

    /**
     * @brief Access the optional content hash.
     */
    [[nodiscard]] const std::optional<std::string> &content_hash() const noexcept;

    /**
     * @brief Return true when a content hash is available.
     */
    [[nodiscard]] bool has_content_hash() const noexcept;

    /**
     * @brief Return true when this signature has a path, size and mtime.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true when two signatures describe the same observed state.
     *
     * The path, size and last write time must match. If both signatures have a
     * content hash, the content hash must match too.
     *
     * @param other Signature to compare with.
     * @return True when both signatures match.
     */
    [[nodiscard]] bool matches(const FileSignature &other) const noexcept;

  private:
    std::filesystem::path path_{};
    std::optional<std::uintmax_t> size_{};
    std::optional<std::filesystem::file_time_type> last_write_time_{};
    std::optional<std::string> content_hash_{};
  };

  /**
   * @brief Compute a stable content hash for a regular file.
   *
   * @param path File path.
   * @return Hash string or filesystem/read error.
   */
  [[nodiscard]] cgride::core::Result<std::string> hash_file_contents(
      const std::filesystem::path &path);

} // namespace cgride::cache

#endif // CGRIDE_CACHE_FILE_SIGNATURE_HPP
