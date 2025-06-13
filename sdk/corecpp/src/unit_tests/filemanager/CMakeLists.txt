cmake_minimum_required(VERSION 3.14)

# Unit tests for LeafraFileManager
project(LeafraFileManagerTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Objective-C++ for Apple platforms
if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 17)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
endif()

# Include directories
include_directories(../../../include)

# Common source files for tests
set(COMMON_SOURCES
    ../../../src/logger.cpp
)

# Platform-specific sources
if(APPLE)
    set(PLATFORM_SOURCES
        ../../../src/leafra_filemanager.cpp
    )
    # Set the FileManager source to be compiled as Objective-C++
    set_source_files_properties(../../../src/leafra_filemanager.cpp PROPERTIES
        COMPILE_FLAGS "-x objective-c++"
    )
else()
    set(PLATFORM_SOURCES
        ../../../src/leafra_filemanager.cpp
    )
endif()

set(ALL_SOURCES ${COMMON_SOURCES} ${PLATFORM_SOURCES})

# Add individual test executables
add_executable(test_filemanager_basic 
    test_filemanager_basic.cpp
    ${ALL_SOURCES}
)

add_executable(test_filemanager_storage_types 
    test_filemanager_storage_types.cpp
    ${ALL_SOURCES}
)

add_executable(test_filemanager_operations 
    test_filemanager_operations.cpp
    ${ALL_SOURCES}
)

add_executable(test_filemanager_edge_cases 
    test_filemanager_edge_cases.cpp
    ${ALL_SOURCES}
)

# Platform-specific linking
if(APPLE)
    # Link all targets with the required Apple frameworks
    set(APPLE_LINK_LIBRARIES
        "-framework Foundation"
        "-framework CoreFoundation"
    )
    
    # Apply to all test targets
    target_link_libraries(test_filemanager_basic ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_filemanager_storage_types ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_filemanager_operations ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_filemanager_edge_cases ${APPLE_LINK_LIBRARIES})
    
    # Set deployment targets for iOS compatibility
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # Configure all test targets as iOS app bundles with Info.plist
        set(TEST_TARGETS 
            test_filemanager_basic 
            test_filemanager_storage_types 
            test_filemanager_operations 
            test_filemanager_edge_cases
        )
        
        foreach(TARGET ${TEST_TARGETS})
            set_target_properties(${TARGET} PROPERTIES
                XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0"
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
                XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.leafra.filemanager.${TARGET}"
            )
        endforeach()
    endif()
endif()

# Enable testing
enable_testing()

# Add tests to CTest
add_test(NAME FileManagerBasic COMMAND test_filemanager_basic)
add_test(NAME FileManagerStorageTypes COMMAND test_filemanager_storage_types)
add_test(NAME FileManagerOperations COMMAND test_filemanager_operations)
add_test(NAME FileManagerEdgeCases COMMAND test_filemanager_edge_cases) 