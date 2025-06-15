#pragma once

#include "types.h"
#include <string>
#include <cstdint>
#include <filesystem>

namespace leafra {

/**
 * @brief Storage location types for cross-platform file operations
 */
enum class StorageType {
    AppStorage,      // Private app storage (iOS: Application Support, Android: Internal Storage)
    DocumentStorage  // User-accessible document storage (iOS: Documents, Android: External Documents)
};

/**
 * @brief Cross-platform file manager for SDK file operations
 * 
 * Provides platform-abstracted file operations that automatically handle
 * platform-specific storage locations and conventions.
 */
class LEAFRA_API FileManager {
public:
    /**
     * @brief File information structure
     */
    struct FileInfo {
        std::string name;
        std::string full_path;
        uint64_t size_bytes;
        int64_t creation_time;
        int64_t modification_time;
        bool is_directory;
        
        FileInfo() : size_bytes(0), creation_time(0), modification_time(0), is_directory(false) {}
    };

    /**
     * @brief Create a file in the specified storage location
     * @param storage_type Where to create the file (app storage or document storage)
     * @param relative_path Relative path from the storage root (e.g., "myfile.txt" or "subfolder/data.db")
     * @param data File content to write
     * @param data_size Size of data in bytes
     * @return ResultCode indicating success or failure
     */
    static ResultCode createFile(StorageType storage_type, const std::string& relative_path, 
                                const void* data, size_t data_size);

    /**
     * @brief Delete a file from the specified storage location
     * @param storage_type Storage location to delete from
     * @param relative_path Relative path to the file
     * @return ResultCode indicating success or failure
     */
    static ResultCode deleteFile(StorageType storage_type, const std::string& relative_path);

    /**
     * @brief Rename/move a file within the same storage location
     * @param storage_type Storage location
     * @param old_relative_path Current relative path
     * @param new_relative_path New relative path
     * @return ResultCode indicating success or failure
     */
    static ResultCode renameFile(StorageType storage_type, const std::string& old_relative_path, 
                                const std::string& new_relative_path);

    /**
     * @brief Copy a file within the same storage location
     * @param storage_type Storage location
     * @param source_relative_path Source file relative path
     * @param dest_relative_path Destination file relative path
     * @return ResultCode indicating success or failure
     */
    static ResultCode copyFile(StorageType storage_type, const std::string& source_relative_path, 
                              const std::string& dest_relative_path);

    /**
     * @brief Check if a file exists
     * @param storage_type Storage location to check
     * @param relative_path Relative path to the file
     * @return true if file exists, false otherwise
     */
    static bool fileExists(StorageType storage_type, const std::string& relative_path);

    /**
     * @brief Get file information (size, dates, etc.)
     * @param storage_type Storage location
     * @param relative_path Relative path to the file
     * @param file_info Output parameter for file information
     * @return ResultCode indicating success or failure
     */
    static ResultCode getFileInfo(StorageType storage_type, const std::string& relative_path, 
                                 FileInfo& file_info);

    /**
     * @brief Create a directory in the specified storage location
     * @param storage_type Storage location
     * @param relative_path Relative path to the directory
     * @return ResultCode indicating success or failure
     */
    static ResultCode createDirectory(StorageType storage_type, const std::string& relative_path);

    /**
     * @brief Delete a directory (must be empty)
     * @param storage_type Storage location
     * @param relative_path Relative path to the directory
     * @return ResultCode indicating success or failure
     */
    static ResultCode deleteDirectory(StorageType storage_type, const std::string& relative_path);

    /**
     * @brief Get the absolute path for a relative path in the specified storage
     * @param storage_type Storage location
     * @param relative_path Relative path
     * @return Full absolute path on the platform
     */
    static std::string getAbsolutePath(StorageType storage_type, const std::string& relative_path);

    /**
     * @brief Get the base directory for the specified storage type
     * @param storage_type Storage location
     * @return Base directory path for the storage type
     */
    static std::string getStorageBasePath(StorageType storage_type);

private:
    /**
     * @brief Initialize storage directories if they don't exist
     * @param storage_type Storage type to initialize
     * @return true if successful, false otherwise
     */
    static bool initializeStorageDirectory(StorageType storage_type);

    /**
     * @brief Validate relative path (no absolute paths, no parent directory traversal)
     * @param relative_path Path to validate
     * @return true if valid, false otherwise
     */
    static bool isValidRelativePath(const std::string& relative_path);

    /**
     * @brief Ensure parent directories exist for the given path
     * @param full_path Full path to file/directory
     * @return true if parent directories exist or were created successfully
     */
    static bool ensureParentDirectoriesExist(const std::string& full_path);
};

} // namespace leafra 