cmake_minimum_required(VERSION 3.22)

project(
  cgride-cache
  VERSION 0.1.0
  DESCRIPTION "Local cache keys, file signatures and cache entry metadata store for Cgride."
  HOMEPAGE_URL "https://github.com/cgride/cache"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(CGRIDE_CACHE_BUILD_TESTS "Build Cgride cache tests" ON)
option(CGRIDE_CACHE_INSTALL "Install Cgride cache" ON)

if(NOT TARGET cgride::core)
  find_package(cgride-core CONFIG REQUIRED)
endif()

add_library(cgride_cache
  src/cache_key.cpp
  src/file_signature.cpp
  src/cache_entry.cpp
  src/cache_options.cpp
  src/cache_store.cpp
)

add_library(cgride::cache ALIAS cgride_cache)

target_compile_features(cgride_cache
  PUBLIC
    cxx_std_23
)

target_include_directories(cgride_cache
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(cgride_cache
  PUBLIC
    cgride::core
)

set_target_properties(cgride_cache
  PROPERTIES
    OUTPUT_NAME cgride_cache
    EXPORT_NAME cache
    CXX_EXTENSIONS OFF
)

if(CGRIDE_CACHE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(CGRIDE_CACHE_INSTALL)
  install(
    TARGETS cgride_cache
    EXPORT cgride-cache-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(
    EXPORT cgride-cache-targets
    NAMESPACE cgride::
    FILE cgride-cache-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-cache
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cgride-cache-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cgride-cache-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-cache
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cgride-cache-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/cgride-cache-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/cgride-cache-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-cache
  )
endif()
