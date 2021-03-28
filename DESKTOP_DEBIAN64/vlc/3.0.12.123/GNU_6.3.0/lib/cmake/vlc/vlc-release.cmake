#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

message("===================> set_target_properties of Release " ${BLD_PRJ} ${_IMPORT_PREFIX})
set_property(TARGET ${BLD_PRJ} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(${BLD_PRJ} PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/lib${BLD_PRJ}.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS ${BLD_PRJ} )
list(APPEND _IMPORT_CHECK_FILES_FOR_${BLD_PRJ} "${_IMPORT_PREFIX}/lib/lib${BLD_PRJ}.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
