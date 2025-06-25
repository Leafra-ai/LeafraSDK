cmake_minimum_required(VERSION 3.18)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name
project(LeafraSDKCmdLineExample)

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_OSX_SYSROOT MATCHES "iphoneos")
        set(PLATFORM_NAME "ios")
        message(STATUS "✅ Building sdkcmdline for iOS")
    else()
        set(PLATFORM_NAME "macos")
        message(STATUS "✅ Building sdkcmdline for macOS")
    endif()
else()
    message(FATAL_ERROR "This application currently supports iOS/macOS only")
endif()

# Find the pre-built LeafraCore.framework
set(LEAFRA_FRAMEWORK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/install/${PLATFORM_NAME}/Frameworks/LeafraCore.framework")

if(NOT EXISTS "${LEAFRA_FRAMEWORK_PATH}")
    message(FATAL_ERROR "LeafraCore.framework not found at: ${LEAFRA_FRAMEWORK_PATH}\n"
                        "Please build the SDK first using: cd ../../sdk && ./build.sh --${PLATFORM_NAME} --embedding-fw=coreml")
endif()

message(STATUS "✅ Found LeafraCore.framework at: ${LEAFRA_FRAMEWORK_PATH}")

# Create imported framework target
add_library(LeafraCore::Framework SHARED IMPORTED)
set_target_properties(LeafraCore::Framework PROPERTIES
    IMPORTED_LOCATION "${LEAFRA_FRAMEWORK_PATH}/LeafraCore"
    FRAMEWORK TRUE
)

# Create the executable
add_executable(sdkcmdline sdkcmdline.cpp)

# Add framework headers to include path
target_include_directories(sdkcmdline PRIVATE "${LEAFRA_FRAMEWORK_PATH}/Headers")

# Link against the pre-built framework
target_link_libraries(sdkcmdline PRIVATE LeafraCore::Framework)


# Set framework search path for runtime linking
if(PLATFORM_NAME STREQUAL "macos")
    set_target_properties(sdkcmdline PROPERTIES
        BUILD_RPATH "${LEAFRA_FRAMEWORK_PATH}/.."
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
elseif(PLATFORM_NAME STREQUAL "ios")
    set_target_properties(sdkcmdline PROPERTIES
        BUILD_RPATH "${LEAFRA_FRAMEWORK_PATH}/.."
        INSTALL_RPATH "@executable_path/Frameworks"
    )
endif()

# Add compile definitions for available features and model paths
target_compile_definitions(sdkcmdline PRIVATE 
    LEAFRA_HAS_FAISS=1
    LEAFRA_HAS_LLAMACPP=1
    LEAFRA_HAS_COREML=1
    LEAFRA_HAS_SQLITE=1
    LEAFRA_HAS_SENTENCEPIECE=1
    LEAFRA_HAS_ICU=1
    LEAFRA_HAS_PDFIUM=1
    LEAFRA_MODELS_ROOT="${LEAFRA_FRAMEWORK_PATH}/Resources/models"
)

# Set output directory
set_target_properties(sdkcmdline PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Configuration summary
message(STATUS "✅ LeafraSDK Command Line Example configured successfully")
message(STATUS "🔗 Using pre-built LeafraCore.framework (no source compilation)")
message(STATUS "📱 Platform: ${PLATFORM_NAME}")
message(STATUS "🚀 Ready to build standalone application") 