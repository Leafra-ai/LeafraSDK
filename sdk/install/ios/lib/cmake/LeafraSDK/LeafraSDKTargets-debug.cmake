#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "LeafraCore" for configuration "Debug"
set_property(TARGET LeafraCore APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LeafraCore PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG "SQLite::SQLite3;ICU::core;Apple::CoreML"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/Frameworks/LeafraCore.framework/LeafraCore"
  IMPORTED_SONAME_DEBUG "@rpath/LeafraCore.framework/LeafraCore"
  )

list(APPEND _cmake_import_check_targets LeafraCore )
list(APPEND _cmake_import_check_files_for_LeafraCore "${_IMPORT_PREFIX}/Frameworks/LeafraCore.framework/LeafraCore" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
