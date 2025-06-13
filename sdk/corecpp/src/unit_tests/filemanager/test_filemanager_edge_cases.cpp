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
bool test_empty_and_null_parameters() {
    std::cout << "\n  Testing empty and null parameters..." << std::endl;
    
    // Test empty relative path
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "", "content", 7);
    TEST_ASSERT(result != ResultCode::SUCCESS, "Should reject empty relative path");
    
    // Test null pointer with non-zero size (should succeed - creates empty file)
    result = FileManager::createFile(StorageType::AppStorage, "null_test.txt", nullptr, 10);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should accept null pointer with non-zero size and create empty file");
    
    // Verify empty file was created
    bool null_exists = FileManager::fileExists(StorageType::AppStorage, "null_test.txt");
    TEST_ASSERT(null_exists, "File should exist even with null pointer and non-zero size");
    
    FileManager::FileInfo null_info;
    FileManager::getFileInfo(StorageType::AppStorage, "null_test.txt", null_info);
    TEST_ASSERT(null_info.size_bytes == 0, "File should have zero size even when non-zero size was specified with null pointer");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "null_test.txt");
    
    // Test null pointer with zero size (should succeed - creates empty file)
    result = FileManager::createFile(StorageType::AppStorage, "empty_file.txt", nullptr, 0);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should accept null pointer with zero size");
    
    // Verify empty file was created
    bool exists = FileManager::fileExists(StorageType::AppStorage, "empty_file.txt");
    TEST_ASSERT(exists, "Empty file should exist");
    
    FileManager::FileInfo info;
    FileManager::getFileInfo(StorageType::AppStorage, "empty_file.txt", info);
    TEST_ASSERT(info.size_bytes == 0, "Empty file should have zero size");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "empty_file.txt");
    
    return true;
}

bool test_very_long_paths() {
    std::cout << "\n  Testing very long paths..." << std::endl;
    
    // Create a very long but valid relative path
    std::string long_path = "very/long/nested/directory/structure/with/many/levels/and/subdirectories/";
    long_path += "that/goes/quite/deep/into/the/filesystem/hierarchy/to/test/path/handling/";
    long_path += "final_file.txt";
    
    const char* content = "Long path test";
    ResultCode result = FileManager::createFile(StorageType::AppStorage, long_path, content, strlen(content));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should handle long valid paths");
    
    // Verify file exists
    bool exists = FileManager::fileExists(StorageType::AppStorage, long_path);
    TEST_ASSERT(exists, "File with long path should exist");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, long_path);
    
    return true;
}

bool test_special_characters_in_paths() {
    std::cout << "\n  Testing special characters in paths..." << std::endl;
    
    // Test various special characters that should be allowed
    std::vector<std::string> valid_paths = {
        "file_with_underscores.txt",
        "file-with-dashes.txt",
        "file.with.dots.txt",
        "file with spaces.txt",
        "file(with)parentheses.txt",
        "file[with]brackets.txt",
        "file{with}braces.txt",
        "file@symbol.txt",
        "file#hash.txt",
        "file$dollar.txt",
        "file%percent.txt",
        "file&ampersand.txt",
        "file+plus.txt",
        "file=equals.txt"
    };
    
    const char* content = "Special char test";
    
    for (const auto& path : valid_paths) {
        ResultCode result = FileManager::createFile(StorageType::AppStorage, path, content, strlen(content));
        if (result == ResultCode::SUCCESS) {
            std::cout << "    ✓ Valid path: " << path << std::endl;
            FileManager::deleteFile(StorageType::AppStorage, path);
        } else {
            std::cout << "    ✗ Failed path: " << path << std::endl;
            // Some special characters might not be supported on all platforms
            // This is informational rather than a hard failure
        }
    }
    
    return true;
}

bool test_unicode_filenames() {
    std::cout << "\n  Testing Unicode filenames..." << std::endl;
    
    // Test various Unicode characters
    std::vector<std::string> unicode_paths = {
        "файл.txt",           // Cyrillic
        "文件.txt",           // Chinese
        "ファイル.txt",        // Japanese
        "파일.txt",           // Korean
        "αρχείο.txt",        // Greek
        "файл_测试.txt",      // Mixed scripts
        "emoji_😀_file.txt"   // Emoji (might not work on all systems)
    };
    
    const char* content = "Unicode test";
    
    for (const auto& path : unicode_paths) {
        ResultCode result = FileManager::createFile(StorageType::AppStorage, path, content, strlen(content));
        if (result == ResultCode::SUCCESS) {
            std::cout << "    ✓ Unicode path supported: " << path << std::endl;
            bool exists = FileManager::fileExists(StorageType::AppStorage, path);
            if (exists) {
                FileManager::deleteFile(StorageType::AppStorage, path);
            }
        } else {
            std::cout << "    ✗ Unicode path not supported: " << path << std::endl;
            // Unicode support varies by platform, so this is informational
        }
    }
    
    return true;
}

bool test_large_file_operations() {
    std::cout << "\n  Testing large file operations..." << std::endl;
    
    // Create a moderately large file (1MB)
    const size_t large_size = 1024 * 1024; // 1MB
    std::vector<char> large_data(large_size, 'A');
    
    // Fill with pattern to make it more realistic
    for (size_t i = 0; i < large_size; i += 100) {
        if (i + 10 < large_size) {
            memcpy(&large_data[i], "0123456789", 10);
        }
    }
    
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "large_file.dat", 
                                               large_data.data(), large_size);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create large file");
    
    // Verify file exists and has correct size
    bool exists = FileManager::fileExists(StorageType::AppStorage, "large_file.dat");
    TEST_ASSERT(exists, "Large file should exist");
    
    FileManager::FileInfo info;
    FileManager::getFileInfo(StorageType::AppStorage, "large_file.dat", info);
    TEST_ASSERT(info.size_bytes == large_size, "Large file should have correct size");
    
    // Test copying large file
    result = FileManager::copyFile(StorageType::AppStorage, "large_file.dat", "large_file_copy.dat");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should copy large file");
    
    // Verify copy has same size
    FileManager::FileInfo copy_info;
    FileManager::getFileInfo(StorageType::AppStorage, "large_file_copy.dat", copy_info);
    TEST_ASSERT(copy_info.size_bytes == large_size, "Copied large file should have same size");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "large_file.dat");
    FileManager::deleteFile(StorageType::AppStorage, "large_file_copy.dat");
    
    return true;
}

bool test_concurrent_operations() {
    std::cout << "\n  Testing concurrent-like operations..." << std::endl;
    
    // Simulate rapid file operations (not truly concurrent, but rapid succession)
    const int num_files = 50;
    std::vector<std::string> filenames;
    
    // Create many files rapidly
    for (int i = 0; i < num_files; ++i) {
        std::string filename = "concurrent_" + std::to_string(i) + ".txt";
        filenames.push_back(filename);
        
        std::string content = "File " + std::to_string(i) + " content";
        ResultCode result = FileManager::createFile(StorageType::AppStorage, filename, 
                                                   content.c_str(), content.length());
        TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create file in rapid succession");
    }
    
    // Verify all files exist
    for (const auto& filename : filenames) {
        bool exists = FileManager::fileExists(StorageType::AppStorage, filename);
        TEST_ASSERT(exists, "Rapidly created file should exist");
    }
    
    // Delete all files rapidly
    for (const auto& filename : filenames) {
        ResultCode result = FileManager::deleteFile(StorageType::AppStorage, filename);
        TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should delete file in rapid succession");
    }
    
    // Verify all files are gone
    for (const auto& filename : filenames) {
        bool exists = FileManager::fileExists(StorageType::AppStorage, filename);
        TEST_ASSERT(!exists, "Rapidly deleted file should not exist");
    }
    
    return true;
}

bool test_edge_case_file_sizes() {
    std::cout << "\n  Testing edge case file sizes..." << std::endl;
    
    // Test zero-size file (already tested but worth repeating)
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "zero_size.txt", nullptr, 0);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create zero-size file");
    
    // Test single byte file
    result = FileManager::createFile(StorageType::AppStorage, "one_byte.txt", "A", 1);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create one-byte file");
    
    // Test file with just whitespace
    const char* whitespace = "   \t\n\r   ";
    result = FileManager::createFile(StorageType::AppStorage, "whitespace.txt", whitespace, strlen(whitespace));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create whitespace-only file");
    
    // Verify all files exist with correct sizes
    FileManager::FileInfo zero_info, one_info, white_info;
    
    FileManager::getFileInfo(StorageType::AppStorage, "zero_size.txt", zero_info);
    FileManager::getFileInfo(StorageType::AppStorage, "one_byte.txt", one_info);
    FileManager::getFileInfo(StorageType::AppStorage, "whitespace.txt", white_info);
    
    TEST_ASSERT(zero_info.size_bytes == 0, "Zero-size file should have size 0");
    TEST_ASSERT(one_info.size_bytes == 1, "One-byte file should have size 1");
    TEST_ASSERT(white_info.size_bytes == strlen(whitespace), "Whitespace file should have correct size");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "zero_size.txt");
    FileManager::deleteFile(StorageType::AppStorage, "one_byte.txt");
    FileManager::deleteFile(StorageType::AppStorage, "whitespace.txt");
    
    return true;
}

bool test_path_traversal_security() {
    std::cout << "\n  Testing path traversal security..." << std::endl;
    
    // Test various path traversal attempts (all should fail)
    std::vector<std::string> malicious_paths = {
        "../escape.txt",
        "../../escape.txt",
        "../../../escape.txt",
        "folder/../escape.txt",
        "folder/../../escape.txt",
        "./folder/../escape.txt",
        "folder/../../../escape.txt",
        "..\\escape.txt",  // Windows-style
        "folder\\..\\escape.txt",
        "/absolute/path.txt",
        "\\absolute\\path.txt"
    };
    
    const char* content = "Should not be created";
    
    for (const auto& path : malicious_paths) {
        ResultCode result = FileManager::createFile(StorageType::AppStorage, path, content, strlen(content));
        TEST_ASSERT(result != ResultCode::SUCCESS, ("Should reject malicious path: " + path).c_str());
        
        // Double-check that no file was created
        bool exists = FileManager::fileExists(StorageType::AppStorage, path);
        TEST_ASSERT(!exists, ("Malicious path should not create file: " + path).c_str());
    }
    
    return true;
}

int main() {
    std::cout << "=== FileManager Edge Cases Tests ===" << std::endl;
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all tests
    RUN_TEST(test_empty_and_null_parameters);
    RUN_TEST(test_very_long_paths);
    RUN_TEST(test_special_characters_in_paths);
    RUN_TEST(test_unicode_filenames);
    RUN_TEST(test_large_file_operations);
    RUN_TEST(test_concurrent_operations);
    RUN_TEST(test_edge_case_file_sizes);
    RUN_TEST(test_path_traversal_security);
    
    // Print summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "🎉 All edge case tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ " << failed_tests << " test(s) failed!" << std::endl;
        return 1;
    }
} 