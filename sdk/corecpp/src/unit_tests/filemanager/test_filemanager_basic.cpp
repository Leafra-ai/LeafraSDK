#include "../../../include/leafra/leafra_filemanager.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cstring>

using namespace leafra;

// Test result code to string conversion
const char* test_result_code_to_string(ResultCode code) {
    switch (code) {
        case ResultCode::SUCCESS: return "SUCCESS";
        case ResultCode::ERROR_INVALID_PARAMETER: return "ERROR_INVALID_PARAMETER";
        case ResultCode::ERROR_INITIALIZATION_FAILED: return "ERROR_INITIALIZATION_FAILED";
        case ResultCode::ERROR_PROCESSING_FAILED: return "ERROR_PROCESSING_FAILED";
        case ResultCode::ERROR_NOT_IMPLEMENTED: return "ERROR_NOT_IMPLEMENTED";
        case ResultCode::ERROR_OUT_OF_MEMORY: return "ERROR_OUT_OF_MEMORY";
        default: return "UNKNOWN";
    }
}

// Simple test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "FAIL: " << message << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "FAIL: " << message << " - Expected: " << (expected) \
                      << ", Actual: " << (actual) << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_RESULT_CODE(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "FAIL: " << message << " - Expected: " << test_result_code_to_string(expected) \
                      << ", Actual: " << test_result_code_to_string(actual) << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        std::cout << "Running " << #test_func << "... "; \
        if (test_func()) { \
            std::cout << "PASS" << std::endl; \
            passed_tests++; \
        } else { \
            std::cout << "FAIL" << std::endl; \
            failed_tests++; \
        } \
        total_tests++; \
    } while(0)

// Test functions
bool test_create_empty_file() {
    std::cout << "\n  Testing empty file creation..." << std::endl;
    
    // Test creating empty file with null pointer and 0 size
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "test_empty.txt", nullptr, 0);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create empty file with null pointer");
    
    // Verify file exists
    bool exists = FileManager::fileExists(StorageType::AppStorage, "test_empty.txt");
    TEST_ASSERT(exists, "Empty file should exist after creation");
    
    // Get file info to verify size
    FileManager::FileInfo info;
    result = FileManager::getFileInfo(StorageType::AppStorage, "test_empty.txt", info);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should get file info for empty file");
    TEST_ASSERT_EQUAL(static_cast<uint64_t>(0), info.size_bytes, "Empty file should have size 0");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "test_empty.txt");
    
    return true;
}

bool test_create_file_with_content() {
    std::cout << "\n  Testing file creation with content..." << std::endl;
    
    const char* test_content = "Hello, FileManager!";
    size_t content_size = strlen(test_content);
    
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "test_content.txt", 
                                               test_content, content_size);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create file with content");
    
    // Verify file exists
    bool exists = FileManager::fileExists(StorageType::AppStorage, "test_content.txt");
    TEST_ASSERT(exists, "File with content should exist after creation");
    
    // Get file info to verify size
    FileManager::FileInfo info;
    result = FileManager::getFileInfo(StorageType::AppStorage, "test_content.txt", info);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should get file info");
    TEST_ASSERT_EQUAL(static_cast<uint64_t>(content_size), info.size_bytes, "File size should match content size");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "test_content.txt");
    
    return true;
}

bool test_file_exists_functionality() {
    std::cout << "\n  Testing file exists functionality..." << std::endl;
    
    // Test non-existent file
    bool exists = FileManager::fileExists(StorageType::AppStorage, "non_existent_file.txt");
    TEST_ASSERT(!exists, "Non-existent file should return false");
    
    // Create a file and test existence
    const char* content = "test";
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "exists_test.txt", content, 4);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create test file");
    
    exists = FileManager::fileExists(StorageType::AppStorage, "exists_test.txt");
    TEST_ASSERT(exists, "Created file should exist");
    
    // Delete and test non-existence
    result = FileManager::deleteFile(StorageType::AppStorage, "exists_test.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should delete test file");
    
    exists = FileManager::fileExists(StorageType::AppStorage, "exists_test.txt");
    TEST_ASSERT(!exists, "Deleted file should not exist");
    
    return true;
}

bool test_delete_file() {
    std::cout << "\n  Testing file deletion..." << std::endl;
    
    // Create a file to delete
    const char* content = "delete me";
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "delete_test.txt", content, 9);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create file for deletion test");
    
    // Verify it exists
    bool exists = FileManager::fileExists(StorageType::AppStorage, "delete_test.txt");
    TEST_ASSERT(exists, "File should exist before deletion");
    
    // Delete the file
    result = FileManager::deleteFile(StorageType::AppStorage, "delete_test.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should delete file successfully");
    
    // Verify it no longer exists
    exists = FileManager::fileExists(StorageType::AppStorage, "delete_test.txt");
    TEST_ASSERT(!exists, "File should not exist after deletion");
    
    // Test deleting non-existent file (should fail gracefully)
    result = FileManager::deleteFile(StorageType::AppStorage, "non_existent.txt");
    TEST_ASSERT(result != ResultCode::SUCCESS, "Deleting non-existent file should fail");
    
    return true;
}

bool test_get_file_info() {
    std::cout << "\n  Testing file info retrieval..." << std::endl;
    
    const char* content = "File info test content with some length";
    size_t content_size = strlen(content);
    
    // Create test file
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "info_test.txt", content, content_size);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create file for info test");
    
    // Get file info
    FileManager::FileInfo info;
    result = FileManager::getFileInfo(StorageType::AppStorage, "info_test.txt", info);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should get file info successfully");
    
    // Verify file info
    TEST_ASSERT_EQUAL(static_cast<uint64_t>(content_size), info.size_bytes, "File size should match content size");
    TEST_ASSERT(!info.name.empty(), "File name should not be empty");
    TEST_ASSERT(!info.full_path.empty(), "Full path should not be empty");
    TEST_ASSERT(info.creation_time > 0, "Creation time should be set");
    TEST_ASSERT(info.modification_time > 0, "Modification time should be set");
    TEST_ASSERT(!info.is_directory, "File should not be marked as directory");
    
    // Test getting info for non-existent file
    result = FileManager::getFileInfo(StorageType::AppStorage, "non_existent.txt", info);
    TEST_ASSERT(result != ResultCode::SUCCESS, "Getting info for non-existent file should fail");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "info_test.txt");
    
    return true;
}

bool test_binary_file_operations() {
    std::cout << "\n  Testing binary file operations..." << std::endl;
    
    // Create binary data
    unsigned char binary_data[] = {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0xFC};
    size_t data_size = sizeof(binary_data);
    
    // Create binary file
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "binary_test.bin", 
                                               binary_data, data_size);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create binary file");
    
    // Verify file exists and has correct size
    bool exists = FileManager::fileExists(StorageType::AppStorage, "binary_test.bin");
    TEST_ASSERT(exists, "Binary file should exist");
    
    FileManager::FileInfo info;
    result = FileManager::getFileInfo(StorageType::AppStorage, "binary_test.bin", info);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should get binary file info");
    TEST_ASSERT_EQUAL(static_cast<uint64_t>(data_size), info.size_bytes, "Binary file size should match data size");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "binary_test.bin");
    
    return true;
}

bool test_path_validation() {
    std::cout << "\n  Testing path validation..." << std::endl;
    
    // Test invalid paths (should fail)
    ResultCode result;
    
    // Absolute path should fail
    result = FileManager::createFile(StorageType::AppStorage, "/absolute/path.txt", "test", 4);
    TEST_ASSERT(result != ResultCode::SUCCESS, "Absolute path should be rejected");
    
    // Parent directory traversal should fail
    result = FileManager::createFile(StorageType::AppStorage, "../escape.txt", "test", 4);
    TEST_ASSERT(result != ResultCode::SUCCESS, "Parent directory traversal should be rejected");
    
    // Another traversal attempt
    result = FileManager::createFile(StorageType::AppStorage, "folder/../escape.txt", "test", 4);
    TEST_ASSERT(result != ResultCode::SUCCESS, "Complex traversal should be rejected");
    
    // Valid relative paths should work
    result = FileManager::createFile(StorageType::AppStorage, "valid_file.txt", "test", 4);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Valid relative path should work");
    
    result = FileManager::createFile(StorageType::AppStorage, "folder/subfolder/file.txt", "test", 4);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Valid nested path should work");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "valid_file.txt");
    FileManager::deleteFile(StorageType::AppStorage, "folder/subfolder/file.txt");
    
    return true;
}

int main() {
    std::cout << "=== FileManager Basic Tests ===" << std::endl;
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all tests
    RUN_TEST(test_create_empty_file);
    RUN_TEST(test_create_file_with_content);
    RUN_TEST(test_file_exists_functionality);
    RUN_TEST(test_delete_file);
    RUN_TEST(test_get_file_info);
    RUN_TEST(test_binary_file_operations);
    RUN_TEST(test_path_validation);
    
    // Print summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "🎉 All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ " << failed_tests << " test(s) failed!" << std::endl;
        return 1;
    }
} 