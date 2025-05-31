cmake_minimum_required(VERSION 3.18)

# Android React Native bindings
project(LeafraSDKAndroid)

# Check if we have the necessary source files (to be created later)
set(RN_ANDROID_SOURCES
    # Native module sources will be added here when implemented
    # LeafraSDKModule.cpp
    # LeafraSDKModule.h
    README.md  # Placeholder for now
)

# Only create the library if we have actual source files
# For now, this is a placeholder configuration

# Create the Android module library (when sources are available)
# add_library(LeafraSDKAndroid SHARED ${RN_ANDROID_SOURCES})

# Link with core library
# target_link_libraries(LeafraSDKAndroid PRIVATE LeafraCore)

# Include directories
# target_include_directories(LeafraSDKAndroid 
#     PUBLIC 
#         ${CMAKE_CURRENT_SOURCE_DIR}
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/../../corecpp/include
# )

# Android specific settings
if(ANDROID)
    # Link with Android log library
    # target_link_libraries(LeafraSDKAndroid PRIVATE log)
    
    # ICU support will be automatically inherited from LeafraCore
    # when the library is created
    
    message(STATUS "📱 Android React Native bindings configured (placeholder)")
    message(STATUS "   Note: Actual implementation files need to be created")
    
    # Set minimum Android API level for ICU support
    if(ANDROID_PLATFORM_LEVEL GREATER_EQUAL 31)
        message(STATUS "✅ Android API ${ANDROID_PLATFORM_LEVEL} supports ICU")
    else()
        message(STATUS "⚠️  Android API ${ANDROID_PLATFORM_LEVEL} - ICU support limited")
        message(STATUS "   Consider using API level 31+ for full ICU support")
    endif()
endif()

# Export configuration for the main build system
set(LEAFRA_RN_ANDROID_CONFIGURED TRUE PARENT_SCOPE)

# Install targets (when library is created)
# install(TARGETS LeafraSDKAndroid
#     EXPORT LeafraSDKTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
# ) 