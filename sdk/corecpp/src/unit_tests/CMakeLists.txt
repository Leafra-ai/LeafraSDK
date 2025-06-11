cmake_minimum_required(VERSION 3.18)

# Unit tests for LeafraSDK
project(LeafraSDKTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories for different test suites
add_subdirectory(chunker)
add_subdirectory(coreml)

# You can add more test subdirectories here in the future
# add_subdirectory(sqlite)
# add_subdirectory(sentencepiece)
# add_subdirectory(parsing) 