cmake_minimum_required(VERSION 3.18)

# Add SDK path
set(LEAFRA_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../sdk")

# Include the SDK
add_subdirectory(${LEAFRA_SDK_PATH} ${CMAKE_CURRENT_BINARY_DIR}/leafra-sdk)

# Link the app with the SDK
target_link_libraries(${PROJECT_NAME} PRIVATE LeafraCore)

# Include SDK headers
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${LEAFRA_SDK_PATH}/corecpp/include
) 