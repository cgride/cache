function(cgride_cache_add_test name source)
  add_executable(${name}
    ${source}
  )

  target_link_libraries(${name}
    PRIVATE
      cgride::cache
  )

  target_compile_features(${name}
    PRIVATE
      cxx_std_23
  )

  set_target_properties(${name}
    PROPERTIES
      CXX_EXTENSIONS OFF
  )

  add_test(
    NAME ${name}
    COMMAND ${name}
  )
endfunction()

cgride_cache_add_test(cgride_cache_cache_key_test cache_key_test.cpp)
cgride_cache_add_test(cgride_cache_file_signature_test file_signature_test.cpp)
cgride_cache_add_test(cgride_cache_cache_entry_test cache_entry_test.cpp)
cgride_cache_add_test(cgride_cache_cache_options_test cache_options_test.cpp)
cgride_cache_add_test(cgride_cache_cache_store_test cache_store_test.cpp)
