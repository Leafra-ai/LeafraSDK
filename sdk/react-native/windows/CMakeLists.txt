cmake_minimum_required(VERSION 3.18)

# Windows React Native bindings
project(LeafraSDKWindows)

# Check if we have the necessary source files (to be created later)
set(RN_WINDOWS_SOURCES
    # Native module sources will be added here when implemented
    # LeafraSDKModule.cpp
    # LeafraSDKModule.h
    README.md  # Placeholder for now
)

# Only create the library if we have actual source files
# For now, this is a placeholder configuration

# Create the Windows module library (when sources are available)
# add_library(LeafraSDKWindows SHARED ${RN_WINDOWS_SOURCES})

# Link with core library
# target_link_libraries(LeafraSDKWindows PRIVATE LeafraCore)

# Include directories
# target_include_directories(LeafraSDKWindows 
#     PUBLIC 
#         ${CMAKE_CURRENT_SOURCE_DIR}
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/../../corecpp/include
# )

# Windows specific settings
if(WIN32)
    # ICU support will be automatically inherited from LeafraCore
    # when the library is created
    
    # Determine architecture for ICU path validation
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WINDOWS_ARCH "x64")
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(WINDOWS_ARCH "arm64")
    else()
        set(WINDOWS_ARCH "x32")
    endif()
    
    message(STATUS "🖥️  Windows React Native bindings configured (placeholder)")
    message(STATUS "   Note: Actual implementation files need to be created")
    message(STATUS "   Architecture: ${WINDOWS_ARCH}")
    
    # Check if ICU libraries are available for this architecture
    set(ICU_WINDOWS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../corecpp/third_party/prebuilt/icu/windows/win-${WINDOWS_ARCH}")
    if(EXISTS "${ICU_WINDOWS_PATH}")
        message(STATUS "✅ ICU libraries available for Windows ${WINDOWS_ARCH}")
    else()
        message(STATUS "⚠️  ICU libraries not found for Windows ${WINDOWS_ARCH}")
        message(STATUS "   Expected path: ${ICU_WINDOWS_PATH}")
    endif()
endif()

# Export configuration for the main build system
set(LEAFRA_RN_WINDOWS_CONFIGURED TRUE PARENT_SCOPE)

# Install targets (when library is created)
# install(TARGETS LeafraSDKWindows
#     EXPORT LeafraSDKTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
# ) 