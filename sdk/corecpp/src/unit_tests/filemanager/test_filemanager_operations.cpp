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
bool test_file_copy_operations() {
    std::cout << "\n  Testing file copy operations..." << std::endl;
    
    const char* original_content = "Original file content for copy test";
    
    // Create source file
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "copy_source.txt", 
                                               original_content, strlen(original_content));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create source file");
    
    // Copy the file
    result = FileManager::copyFile(StorageType::AppStorage, "copy_source.txt", "copy_dest.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should copy file successfully");
    
    // Verify both files exist
    bool source_exists = FileManager::fileExists(StorageType::AppStorage, "copy_source.txt");
    bool dest_exists = FileManager::fileExists(StorageType::AppStorage, "copy_dest.txt");
    
    TEST_ASSERT(source_exists, "Source file should still exist after copy");
    TEST_ASSERT(dest_exists, "Destination file should exist after copy");
    
    // Verify file sizes are the same
    FileManager::FileInfo source_info, dest_info;
    FileManager::getFileInfo(StorageType::AppStorage, "copy_source.txt", source_info);
    FileManager::getFileInfo(StorageType::AppStorage, "copy_dest.txt", dest_info);
    
    TEST_ASSERT(source_info.size_bytes == dest_info.size_bytes, "Copied file should have same size");
    
    // Test copying to nested directory
    result = FileManager::copyFile(StorageType::AppStorage, "copy_source.txt", "nested/copy_nested.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should copy to nested directory");
    
    bool nested_exists = FileManager::fileExists(StorageType::AppStorage, "nested/copy_nested.txt");
    TEST_ASSERT(nested_exists, "Nested copy should exist");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "copy_source.txt");
    FileManager::deleteFile(StorageType::AppStorage, "copy_dest.txt");
    FileManager::deleteFile(StorageType::AppStorage, "nested/copy_nested.txt");
    
    return true;
}

bool test_file_rename_operations() {
    std::cout << "\n  Testing file rename operations..." << std::endl;
    
    const char* content = "File content for rename test";
    
    // Create source file
    ResultCode result = FileManager::createFile(StorageType::AppStorage, "rename_source.txt", 
                                               content, strlen(content));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create source file");
    
    // Rename the file
    result = FileManager::renameFile(StorageType::AppStorage, "rename_source.txt", "rename_dest.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should rename file successfully");
    
    // Verify old file doesn't exist and new file exists
    bool old_exists = FileManager::fileExists(StorageType::AppStorage, "rename_source.txt");
    bool new_exists = FileManager::fileExists(StorageType::AppStorage, "rename_dest.txt");
    
    TEST_ASSERT(!old_exists, "Old file should not exist after rename");
    TEST_ASSERT(new_exists, "New file should exist after rename");
    
    // Verify content is preserved
    FileManager::FileInfo info;
    FileManager::getFileInfo(StorageType::AppStorage, "rename_dest.txt", info);
    TEST_ASSERT(info.size_bytes == strlen(content), "File size should be preserved after rename");
    
    // Test renaming to nested directory (move operation)
    result = FileManager::renameFile(StorageType::AppStorage, "rename_dest.txt", "moved/renamed_file.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should move file to nested directory");
    
    bool moved_exists = FileManager::fileExists(StorageType::AppStorage, "moved/renamed_file.txt");
    bool old_location_exists = FileManager::fileExists(StorageType::AppStorage, "rename_dest.txt");
    
    TEST_ASSERT(moved_exists, "Moved file should exist");
    TEST_ASSERT(!old_location_exists, "File should not exist in old location");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "moved/renamed_file.txt");
    
    return true;
}

bool test_directory_operations() {
    std::cout << "\n  Testing directory operations..." << std::endl;
    
    // Create directory
    ResultCode result = FileManager::createDirectory(StorageType::AppStorage, "test_directory");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create directory");
    
    // Create nested directory
    result = FileManager::createDirectory(StorageType::AppStorage, "parent/child/grandchild");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create nested directories");
    
    // Create file in directory
    const char* content = "File in directory";
    result = FileManager::createFile(StorageType::AppStorage, "test_directory/file_in_dir.txt", 
                                    content, strlen(content));
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should create file in directory");
    
    bool file_exists = FileManager::fileExists(StorageType::AppStorage, "test_directory/file_in_dir.txt");
    TEST_ASSERT(file_exists, "File should exist in directory");
    
    // Try to delete non-empty directory (should fail)
    result = FileManager::deleteDirectory(StorageType::AppStorage, "test_directory");
    TEST_ASSERT(result != ResultCode::SUCCESS, "Should not delete non-empty directory");
    
    // Delete file first, then directory
    FileManager::deleteFile(StorageType::AppStorage, "test_directory/file_in_dir.txt");
    result = FileManager::deleteDirectory(StorageType::AppStorage, "test_directory");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should delete empty directory");
    
    // Clean up nested directories
    FileManager::deleteDirectory(StorageType::AppStorage, "parent/child/grandchild");
    FileManager::deleteDirectory(StorageType::AppStorage, "parent/child");
    FileManager::deleteDirectory(StorageType::AppStorage, "parent");
    
    return true;
}

bool test_copy_error_conditions() {
    std::cout << "\n  Testing copy error conditions..." << std::endl;
    
    // Try to copy non-existent file
    ResultCode result = FileManager::copyFile(StorageType::AppStorage, "non_existent.txt", "dest.txt");
    TEST_ASSERT(result != ResultCode::SUCCESS, "Should fail to copy non-existent file");
    
    // Create source file
    const char* content = "Source content";
    FileManager::createFile(StorageType::AppStorage, "copy_error_source.txt", content, strlen(content));
    
    // Try to copy to invalid path
    result = FileManager::copyFile(StorageType::AppStorage, "copy_error_source.txt", "../invalid.txt");
    TEST_ASSERT(result != ResultCode::SUCCESS, "Should fail to copy to invalid path");
    
    // Try to copy to same file (should fail or handle gracefully)
    result = FileManager::copyFile(StorageType::AppStorage, "copy_error_source.txt", "copy_error_source.txt");
    TEST_ASSERT(result != ResultCode::SUCCESS, "Should handle copying to same file");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "copy_error_source.txt");
    
    return true;
}

bool test_rename_error_conditions() {
    std::cout << "\n  Testing rename error conditions..." << std::endl;
    
    // Try to rename non-existent file
    ResultCode result = FileManager::renameFile(StorageType::AppStorage, "non_existent.txt", "new_name.txt");
    TEST_ASSERT(result != ResultCode::SUCCESS, "Should fail to rename non-existent file");
    
    // Create source file
    const char* content = "Rename test content";
    FileManager::createFile(StorageType::AppStorage, "rename_error_source.txt", content, strlen(content));
    
    // Try to rename to invalid path
    result = FileManager::renameFile(StorageType::AppStorage, "rename_error_source.txt", "../invalid.txt");
    TEST_ASSERT(result != ResultCode::SUCCESS, "Should fail to rename to invalid path");
    
    // Try to rename to same name (should succeed or handle gracefully)
    result = FileManager::renameFile(StorageType::AppStorage, "rename_error_source.txt", "rename_error_source.txt");
    // This might succeed (no-op) or fail depending on implementation
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "rename_error_source.txt");
    
    return true;
}

bool test_complex_file_operations() {
    std::cout << "\n  Testing complex file operations..." << std::endl;
    
    const char* content1 = "First file content";
    const char* content2 = "Second file content";
    
    // Create multiple files
    FileManager::createFile(StorageType::AppStorage, "complex/file1.txt", content1, strlen(content1));
    FileManager::createFile(StorageType::AppStorage, "complex/file2.txt", content2, strlen(content2));
    
    // Copy file1 to backup
    ResultCode result = FileManager::copyFile(StorageType::AppStorage, "complex/file1.txt", "complex/file1_backup.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should copy file1 to backup");
    
    // Rename file2
    result = FileManager::renameFile(StorageType::AppStorage, "complex/file2.txt", "complex/file2_renamed.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should rename file2");
    
    // Verify all files exist
    bool file1_exists = FileManager::fileExists(StorageType::AppStorage, "complex/file1.txt");
    bool backup_exists = FileManager::fileExists(StorageType::AppStorage, "complex/file1_backup.txt");
    bool renamed_exists = FileManager::fileExists(StorageType::AppStorage, "complex/file2_renamed.txt");
    bool old_file2_exists = FileManager::fileExists(StorageType::AppStorage, "complex/file2.txt");
    
    TEST_ASSERT(file1_exists, "Original file1 should exist");
    TEST_ASSERT(backup_exists, "Backup file should exist");
    TEST_ASSERT(renamed_exists, "Renamed file should exist");
    TEST_ASSERT(!old_file2_exists, "Old file2 should not exist");
    
    // Copy renamed file to another location
    result = FileManager::copyFile(StorageType::AppStorage, "complex/file2_renamed.txt", "archive/file2_copy.txt");
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Should copy to archive directory");
    
    // Verify archive copy exists
    bool archive_exists = FileManager::fileExists(StorageType::AppStorage, "archive/file2_copy.txt");
    TEST_ASSERT(archive_exists, "Archive copy should exist");
    
    // Clean up
    FileManager::deleteFile(StorageType::AppStorage, "complex/file1.txt");
    FileManager::deleteFile(StorageType::AppStorage, "complex/file1_backup.txt");
    FileManager::deleteFile(StorageType::AppStorage, "complex/file2_renamed.txt");
    FileManager::deleteFile(StorageType::AppStorage, "archive/file2_copy.txt");
    
    return true;
}

int main() {
    std::cout << "=== FileManager Operations Tests ===" << std::endl;
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all tests
    RUN_TEST(test_file_copy_operations);
    RUN_TEST(test_file_rename_operations);
    RUN_TEST(test_directory_operations);
    RUN_TEST(test_copy_error_conditions);
    RUN_TEST(test_rename_error_conditions);
    RUN_TEST(test_complex_file_operations);
    
    // Print summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "🎉 All operations tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ " << failed_tests << " test(s) failed!" << std::endl;
        return 1;
    }
} 