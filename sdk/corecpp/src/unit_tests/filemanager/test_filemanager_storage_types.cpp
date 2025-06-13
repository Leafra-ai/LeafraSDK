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
bool test_app_storage_operations() {
    std::cout << "\n  Testing AppStorage operations..." << std::endl;
    
    const char* content = "App storage test content";
    size_t content_size = strlen(content);
    
    // Create file in app storage
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "app_test.txt", content, content_size);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create file in app storage");
    
    // Verify file exists
    bool exists = FileManager::fileExists(StorageType::AppStorage, "app_test.txt");
    TEST_ASSERT(exists, "File should exist in app storage");
    
    // Get file info
    FileManager::FileInfo info;
    result = FileManager::getFileInfo(StorageType::AppStorage, "app_test.txt", info);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should get file info from app storage");
    
    // Verify the path contains app-specific directory
    std::cout << "    App storage path: " << info.full_path << std::endl;
    TEST_ASSERT(!info.full_path.empty(), "App storage path should not be empty");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "app_test.txt");
    
    return true;
}

bool test_document_storage_operations() {
    std::cout << "\n  Testing DocumentStorage operations..." << std::endl;
    
    const char* content = "Document storage test content";
    size_t content_size = strlen(content);
    
    // Create file in document storage
    ResultCode result = FileManager::createFile(StorageType::DocumentStorage, "doc_test.txt", content, content_size);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create file in document storage");
    
    // Verify file exists
    bool exists = FileManager::fileExists(StorageType::DocumentStorage, "doc_test.txt");
    TEST_ASSERT(exists, "File should exist in document storage");
    
    // Get file info
    FileManager::FileInfo info;
    result = FileManager::getFileInfo(StorageType::DocumentStorage, "doc_test.txt", info);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should get file info from document storage");
    
    // Verify the path is different from app storage
    std::cout << "    Document storage path: " << info.full_path << std::endl;
    TEST_ASSERT(!info.full_path.empty(), "Document storage path should not be empty");
    
    // Clean up
    FileManager::deleteFile(StorageType::DocumentStorage, "doc_test.txt");
    
    return true;
}

bool test_storage_isolation() {
    std::cout << "\n  Testing storage isolation..." << std::endl;
    
    const char* app_content = "App storage content";
    const char* doc_content = "Document storage content";
    
    // Create files with same name in different storages
    ResultCode result1 = FileManager::createFile(StorageType::AppStorage, "isolation_test.txt", 
                                                 app_content, strlen(app_content));
    ResultCode result2 = FileManager::createFile(StorageType::DocumentStorage, "isolation_test.txt", 
                                                 doc_content, strlen(doc_content));
    
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result1, "Should create file in app storage");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result2, "Should create file in document storage");
    
    // Both files should exist independently
    bool app_exists = FileManager::fileExists(StorageType::AppStorage, "isolation_test.txt");
    bool doc_exists = FileManager::fileExists(StorageType::DocumentStorage, "isolation_test.txt");
    
    TEST_ASSERT(app_exists, "App storage file should exist");
    TEST_ASSERT(doc_exists, "Document storage file should exist");
    
    // Get paths to verify they're different
    FileManager::FileInfo app_info, doc_info;
    FileManager::getFileInfo(StorageType::AppStorage, "isolation_test.txt", app_info);
    FileManager::getFileInfo(StorageType::DocumentStorage, "isolation_test.txt", doc_info);
    
    TEST_ASSERT(app_info.full_path != doc_info.full_path, "Storage paths should be different");
    std::cout << "    App path: " << app_info.full_path << std::endl;
    std::cout << "    Doc path: " << doc_info.full_path << std::endl;
    
    // Delete from one storage shouldn't affect the other
    FileManager::deleteFile(StorageType::AppStorage, "isolation_test.txt");
    
    app_exists = FileManager::fileExists(StorageType::AppStorage, "isolation_test.txt");
    doc_exists = FileManager::fileExists(StorageType::DocumentStorage, "isolation_test.txt");
    
    TEST_ASSERT(!app_exists, "App storage file should be deleted");
    TEST_ASSERT(doc_exists, "Document storage file should still exist");
    
    // Clean up
    FileManager::deleteFile(StorageType::DocumentStorage, "isolation_test.txt");
    
    return true;
}

bool test_storage_base_paths() {
    std::cout << "\n  Testing storage base paths..." << std::endl;
    
    // Get base paths for both storage types
    std::string app_base = FileManager::getStorageBasePath(StorageType::AppStorage);
    std::string doc_base = FileManager::getStorageBasePath(StorageType::DocumentStorage);
    
    TEST_ASSERT(!app_base.empty(), "App storage base path should not be empty");
    TEST_ASSERT(!doc_base.empty(), "Document storage base path should not be empty");
    TEST_ASSERT(app_base != doc_base, "Storage base paths should be different");
    
    std::cout << "    App storage base: " << app_base << std::endl;
    std::cout << "    Document storage base: " << doc_base << std::endl;
    
    // Test absolute path generation
    std::string app_abs = FileManager::getAbsolutePath(StorageType::AppStorage, "test.txt");
    std::string doc_abs = FileManager::getAbsolutePath(StorageType::DocumentStorage, "test.txt");
    
    TEST_ASSERT(!app_abs.empty(), "App storage absolute path should not be empty");
    TEST_ASSERT(!doc_abs.empty(), "Document storage absolute path should not be empty");
    TEST_ASSERT(app_abs != doc_abs, "Absolute paths should be different");
    
    // Verify paths contain the base paths
    TEST_ASSERT(app_abs.find(app_base) == 0, "App absolute path should start with app base");
    TEST_ASSERT(doc_abs.find(doc_base) == 0, "Doc absolute path should start with doc base");
    
    return true;
}

bool test_nested_directories_in_storages() {
    std::cout << "\n  Testing nested directories in different storages..." << std::endl;
    
    const char* content = "Nested directory test";
    
    // Test nested directories in app storage
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "level1/level2/app_nested.txt", 
                                               content, strlen(content));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create nested file in app storage");
    
    bool exists = FileManager::fileExists(StorageType::AppStorage, "level1/level2/app_nested.txt");
    TEST_ASSERT(exists, "Nested file should exist in app storage");
    
    // Test nested directories in document storage
    result = FileManager::createFile(StorageType::DocumentStorage, "docs/projects/doc_nested.txt", 
                                    content, strlen(content));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create nested file in document storage");
    
    exists = FileManager::fileExists(StorageType::DocumentStorage, "docs/projects/doc_nested.txt");
    TEST_ASSERT(exists, "Nested file should exist in document storage");
    
    // Verify paths are different
    FileManager::FileInfo app_info, doc_info;
    FileManager::getFileInfo(StorageType::AppStorage, "level1/level2/app_nested.txt", app_info);
    FileManager::getFileInfo(StorageType::DocumentStorage, "docs/projects/doc_nested.txt", doc_info);
    
    TEST_ASSERT(app_info.full_path != doc_info.full_path, "Nested file paths should be different");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "level1/level2/app_nested.txt");
    FileManager::deleteFile(StorageType::DocumentStorage, "docs/projects/doc_nested.txt");
    
    return true;
}

bool test_cross_storage_operations() {
    std::cout << "\n  Testing cross-storage operations..." << std::endl;
    
    const char* content = "Cross storage test";
    
    // Create file in app storage
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "source.txt", content, strlen(content));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create source file in app storage");
    
    // Try to copy to document storage (this should work within same storage only)
    // Note: Our current API doesn't support cross-storage copy, so this tests the limitation
    result = FileManager::copyFile(StorageType::AppStorage, "source.txt", "copy_in_app.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should copy within same storage");
    
    // Verify both files exist in app storage
    bool source_exists = FileManager::fileExists(StorageType::AppStorage, "source.txt");
    bool copy_exists = FileManager::fileExists(StorageType::AppStorage, "copy_in_app.txt");
    
    TEST_ASSERT(source_exists, "Source file should exist");
    TEST_ASSERT(copy_exists, "Copy file should exist");
    
    // Verify copy doesn't exist in document storage (as expected)
    bool doc_copy_exists = FileManager::fileExists(StorageType::DocumentStorage, "copy_in_app.txt");
    TEST_ASSERT(!doc_copy_exists, "Copy should not exist in different storage");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "source.txt");
    FileManager::deleteFile(StorageType::AppStorage, "copy_in_app.txt");
    
    return true;
}

int main() {
    std::cout << "=== FileManager Storage Types Tests ===" << std::endl;
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all tests
    RUN_TEST(test_app_storage_operations);
    RUN_TEST(test_document_storage_operations);
    RUN_TEST(test_storage_isolation);
    RUN_TEST(test_storage_base_paths);
    RUN_TEST(test_nested_directories_in_storages);
    RUN_TEST(test_cross_storage_operations);
    
    // Print summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "🎉 All storage type tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ " << failed_tests << " test(s) failed!" << std::endl;
        return 1;
    }
} 