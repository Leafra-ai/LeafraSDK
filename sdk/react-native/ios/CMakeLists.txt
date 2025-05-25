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
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../corecpp/include
)

# iOS specific settings
if(LEAFRA_PLATFORM_IOS)
    set_target_properties(LeafraSDKiOS PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.1"
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