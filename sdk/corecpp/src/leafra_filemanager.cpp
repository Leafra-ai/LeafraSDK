#include "leafra/leafra_filemanager.h"
#include "leafra/logger.h"
#include <fstream>
#include <filesystem>
#include <cstring>

#ifdef __APPLE__
    #include <Foundation/Foundation.h>
    #include <TargetConditionals.h>
#endif

namespace leafra {

// ==============================================================================
// Platform-specific storage path resolution
// ==============================================================================

std::string FileManager::getStorageBasePath(StorageType storage_type) {
#ifdef __APPLE__
    NSArray *paths = nil;
    NSString *basePath = nil;
    
    switch (storage_type) {
        case StorageType::AppStorage:
            // iOS/macOS: Application Support directory
            paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
            if ([paths count] > 0) {
                basePath = [paths objectAtIndex:0];
                // Append app-specific subdirectory
                NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
                if (bundleId) {
                    basePath = [basePath stringByAppendingPathComponent:bundleId];
                } else {
                    basePath = [basePath stringByAppendingPathComponent:@"LeafraSDK"];
                }
            }
            break;
            
        case StorageType::DocumentStorage:
            // iOS/macOS: Documents directory
            paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            if ([paths count] > 0) {
                basePath = [paths objectAtIndex:0];
            }
            break;
    }
    
    if (basePath) {
        return std::string([basePath UTF8String]);
    }
    
    LEAFRA_ERROR() << "Failed to get storage base path for storage type: " << static_cast<int>(storage_type);
    return "";
    
#elif defined(_WIN32)
    // Windows implementation (placeholder for future)
    LEAFRA_ERROR() << "Windows file manager not implemented yet";
    return "";
    
#elif defined(__ANDROID__)
    // Android implementation (placeholder for future)
    LEAFRA_ERROR() << "Android file manager not implemented yet";
    return "";
    
#else
    // Linux implementation (placeholder for future)
    LEAFRA_ERROR() << "Linux file manager not implemented yet";
    return "";
#endif
}

std::string FileManager::getAbsolutePath(StorageType storage_type, const std::string& relative_path) {
    if (!isValidRelativePath(relative_path)) {
        LEAFRA_ERROR() << "Invalid relative path: " << relative_path;
        return "";
    }
    
    std::string base_path = getStorageBasePath(storage_type);
    if (base_path.empty()) {
        return "";
    }
    
    std::filesystem::path full_path = std::filesystem::path(base_path) / relative_path;
    return full_path.string();
}

// ==============================================================================
// File operations
// ==============================================================================

ResultCode FileManager::createFile(StorageType storage_type, const std::string& relative_path, 
                                  const void* data, size_t data_size) {
    LEAFRA_DEBUG() << "Creating file: " << relative_path << " (size: " << data_size << ") in storage type: " << static_cast<int>(storage_type);
    
    std::string full_path = getAbsolutePath(storage_type, relative_path);
    if (full_path.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    // Initialize storage directory if needed
    if (!initializeStorageDirectory(storage_type)) {
        LEAFRA_ERROR() << "Failed to initialize storage directory";
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    // Ensure parent directories exist
    if (!ensureParentDirectoriesExist(full_path)) {
        LEAFRA_ERROR() << "Failed to create parent directories for: " << full_path;
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    // Create and write the file
    std::ofstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
        LEAFRA_ERROR() << "Failed to create file: " << full_path;
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    // Write data if provided, otherwise create empty file (0 size)
    // Accepts null pointer and 0 size to create empty files
    if (data != nullptr && data_size > 0) {
        file.write(static_cast<const char*>(data), data_size);
        if (file.fail()) {
            LEAFRA_ERROR() << "Failed to write data to file: " << full_path;
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
    }
    
    file.close();
    LEAFRA_DEBUG() << "File created successfully: " << full_path << " (size: " << data_size << ")";
    return ResultCode::SUCCESS;
}

ResultCode FileManager::deleteFile(StorageType storage_type, const std::string& relative_path) {
    LEAFRA_DEBUG() << "Deleting file: " << relative_path;
    
    std::string full_path = getAbsolutePath(storage_type, relative_path);
    if (full_path.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    std::error_code ec;
    if (std::filesystem::remove(full_path, ec)) {
        LEAFRA_DEBUG() << "File deleted successfully: " << full_path;
        return ResultCode::SUCCESS;
    } else {
        LEAFRA_ERROR() << "Failed to delete file: " << full_path << " Error: " << ec.message();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FileManager::renameFile(StorageType storage_type, const std::string& old_relative_path, 
                                  const std::string& new_relative_path) {
    LEAFRA_DEBUG() << "Renaming file from: " << old_relative_path << " to: " << new_relative_path;
    
    std::string old_full_path = getAbsolutePath(storage_type, old_relative_path);
    std::string new_full_path = getAbsolutePath(storage_type, new_relative_path);
    
    if (old_full_path.empty() || new_full_path.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    // Ensure parent directories exist for the new path
    if (!ensureParentDirectoriesExist(new_full_path)) {
        LEAFRA_ERROR() << "Failed to create parent directories for: " << new_full_path;
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    std::error_code ec;
    std::filesystem::rename(old_full_path, new_full_path, ec);
    if (!ec) {
        LEAFRA_DEBUG() << "File renamed successfully: " << old_full_path << " -> " << new_full_path;
        return ResultCode::SUCCESS;
    } else {
        LEAFRA_ERROR() << "Failed to rename file: " << ec.message();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FileManager::copyFile(StorageType storage_type, const std::string& source_relative_path, 
                                const std::string& dest_relative_path) {
    LEAFRA_DEBUG() << "Copying file from: " << source_relative_path << " to: " << dest_relative_path;
    
    std::string source_full_path = getAbsolutePath(storage_type, source_relative_path);
    std::string dest_full_path = getAbsolutePath(storage_type, dest_relative_path);
    
    if (source_full_path.empty() || dest_full_path.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    // Ensure parent directories exist for the destination
    if (!ensureParentDirectoriesExist(dest_full_path)) {
        LEAFRA_ERROR() << "Failed to create parent directories for: " << dest_full_path;
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    std::error_code ec;
    std::filesystem::copy_file(source_full_path, dest_full_path, ec);
    if (!ec) {
        LEAFRA_DEBUG() << "File copied successfully: " << source_full_path << " -> " << dest_full_path;
        return ResultCode::SUCCESS;
    } else {
        LEAFRA_ERROR() << "Failed to copy file: " << ec.message();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

bool FileManager::fileExists(StorageType storage_type, const std::string& relative_path) {
    std::string full_path = getAbsolutePath(storage_type, relative_path);
    if (full_path.empty()) {
        return false;
    }
    
    return std::filesystem::exists(full_path);
}

ResultCode FileManager::getFileInfo(StorageType storage_type, const std::string& relative_path, 
                                   FileInfo& file_info) {
    std::string full_path = getAbsolutePath(storage_type, relative_path);
    if (full_path.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    std::error_code ec;
    if (!std::filesystem::exists(full_path, ec)) {
        LEAFRA_ERROR() << "File does not exist: " << full_path;
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    // Get file status
    auto file_status = std::filesystem::status(full_path, ec);
    if (ec) {
        LEAFRA_ERROR() << "Failed to get file status: " << ec.message();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    // Fill file info
    file_info.name = std::filesystem::path(relative_path).filename().string();
    file_info.full_path = full_path;
    file_info.is_directory = std::filesystem::is_directory(file_status);
    
    if (!file_info.is_directory) {
        file_info.size_bytes = std::filesystem::file_size(full_path, ec);
        if (ec) {
            LEAFRA_WARNING() << "Failed to get file size: " << ec.message();
            file_info.size_bytes = 0;
        }
    }
    
    // Get file times
    auto last_write_time = std::filesystem::last_write_time(full_path, ec);
    if (!ec) {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            last_write_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        file_info.modification_time = std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count();
        file_info.creation_time = file_info.modification_time; // Fallback to modification time
    }
    
    return ResultCode::SUCCESS;
}

// ==============================================================================
// Directory operations
// ==============================================================================

ResultCode FileManager::createDirectory(StorageType storage_type, const std::string& relative_path) {
    LEAFRA_DEBUG() << "Creating directory: " << relative_path;
    
    std::string full_path = getAbsolutePath(storage_type, relative_path);
    if (full_path.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    // Initialize storage directory if needed
    if (!initializeStorageDirectory(storage_type)) {
        LEAFRA_ERROR() << "Failed to initialize storage directory";
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
    
    std::error_code ec;
    if (std::filesystem::create_directories(full_path, ec)) {
        LEAFRA_DEBUG() << "Directory created successfully: " << full_path;
        return ResultCode::SUCCESS;
    } else if (std::filesystem::exists(full_path)) {
        LEAFRA_DEBUG() << "Directory already exists: " << full_path;
        return ResultCode::SUCCESS;
    } else {
        LEAFRA_ERROR() << "Failed to create directory: " << full_path << " Error: " << ec.message();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FileManager::deleteDirectory(StorageType storage_type, const std::string& relative_path) {
    LEAFRA_DEBUG() << "Deleting directory: " << relative_path;
    
    std::string full_path = getAbsolutePath(storage_type, relative_path);
    if (full_path.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    std::error_code ec;
    if (std::filesystem::remove(full_path, ec)) {
        LEAFRA_DEBUG() << "Directory deleted successfully: " << full_path;
        return ResultCode::SUCCESS;
    } else {
        LEAFRA_ERROR() << "Failed to delete directory: " << full_path << " Error: " << ec.message();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

// ==============================================================================
// Helper functions
// ==============================================================================

bool FileManager::initializeStorageDirectory(StorageType storage_type) {
    std::string base_path = getStorageBasePath(storage_type);
    if (base_path.empty()) {
        return false;
    }
    
    std::error_code ec;
    if (!std::filesystem::exists(base_path, ec)) {
        if (std::filesystem::create_directories(base_path, ec)) {
            LEAFRA_DEBUG() << "Created storage directory: " << base_path;
            return true;
        } else {
            LEAFRA_ERROR() << "Failed to create storage directory: " << base_path << " Error: " << ec.message();
            return false;
        }
    }
    
    return true;
}

bool FileManager::isValidRelativePath(const std::string& relative_path) {
    if (relative_path.empty()) {
        return false;
    }
    
    // Check for absolute path indicators
    if (relative_path[0] == '/' || relative_path[0] == '\\') {
        return false;
    }
    
    // Check for Windows drive letters
    if (relative_path.length() >= 2 && relative_path[1] == ':') {
        return false;
    }
    
    // Check for parent directory traversal
    if (relative_path.find("..") != std::string::npos) {
        return false;
    }
    
    return true;
}

bool FileManager::ensureParentDirectoriesExist(const std::string& full_path) {
    std::filesystem::path path(full_path);
    std::filesystem::path parent_path = path.parent_path();
    
    if (parent_path.empty()) {
        return true;
    }
    
    std::error_code ec;
    if (!std::filesystem::exists(parent_path, ec)) {
        if (std::filesystem::create_directories(parent_path, ec)) {
            LEAFRA_DEBUG() << "Created parent directories: " << parent_path.string();
            return true;
        } else {
            LEAFRA_ERROR() << "Failed to create parent directories: " << parent_path.string() << " Error: " << ec.message();
            return false;
        }
    }
    
    return true;
}

} // namespace leafra 