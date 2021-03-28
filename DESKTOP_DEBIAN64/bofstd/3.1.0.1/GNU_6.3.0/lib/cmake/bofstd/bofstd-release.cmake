#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "bofstd" for configuration "Release"
set_property(TARGET bofstd APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(bofstd PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libbofstd.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS bofstd )
list(APPEND _IMPORT_CHECK_FILES_FOR_bofstd "${_IMPORT_PREFIX}/lib/libbofstd.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
