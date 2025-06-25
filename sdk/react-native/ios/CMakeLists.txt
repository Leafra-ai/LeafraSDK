cmake_minimum_required(VERSION 3.18)

# iOS React Native bindings
set(RN_IOS_SOURCES
    LeafraSDKModule.mm
    LeafraSDKModule.h
    LeafraSDKBridge.mm
    LeafraSDKBridge.h
)

# Create the iOS module library
add_library(LeafraSDKiOS STATIC ${RN_IOS_SOURCES})

# Set target properties
set_target_properties(LeafraSDKiOS PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER com.leafra.sdk.ios
)

# Link with core library
target_link_libraries(LeafraSDKiOS PRIVATE LeafraCore)

# Include directories
target_include_directories(LeafraSDKiOS 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../corecpp/include
)

# Add React Native include paths if available
# Based on project structure: PROJECT_SOURCE_DIR = LeafraSDK/sdk
# React Native is at: LeafraSDK/example/Leafra/node_modules/react-native
set(RN_NODE_MODULES_PATH "${PROJECT_SOURCE_DIR}/../example/Leafra/ios/Pods/Headers/Public")

if(EXISTS "${RN_NODE_MODULES_PATH}")
    target_include_directories(LeafraSDKiOS 
        PRIVATE 
            ${RN_NODE_MODULES_PATH}
            ${RN_NODE_MODULES_PATH}/React-Core
    )
    message(STATUS "✅ React Native headers found at: ${RN_NODE_MODULES_PATH}")
    message(STATUS "   iOS bindings will include RN paths")
else()
    message(STATUS "⚠️  React Native headers not found at: ${RN_NODE_MODULES_PATH}")
    message(STATUS "   iOS bindings will build without React Native integration")
endif()

# iOS specific settings
if(LEAFRA_PLATFORM_IOS)
    set_target_properties(LeafraSDKiOS PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.1"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    )
    # Enable ARC for Objective-C++ files
    target_compile_options(LeafraSDKiOS PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fobjc-arc>
    )
endif()

# macOS specific settings  
if(LEAFRA_PLATFORM_MACOS)
    set_target_properties(LeafraSDKiOS PROPERTIES
        XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "12.0"
    )
endif()

# Install targets
install(TARGETS LeafraSDKiOS
    EXPORT LeafraSDKTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FRAMEWORK DESTINATION Frameworks
) 