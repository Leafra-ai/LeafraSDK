/*
** LeafraSDK SQLite Integration
** Cross-platform SQLite wrapper for database operations
*/

#ifndef LEAFRA_SQLITE_H
#define LEAFRA_SQLITE_H

#ifdef __cplusplus

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>

// Include SQLite headers based on configuration
#ifdef LEAFRA_USE_SYSTEM_SQLITE_HEADERS
    // Use system SQLite headers (iOS/macOS)
    #include <sqlite3.h>
#else
    // Use custom SQLite headers (Android/Windows)
    #include "sqlite3.h"
#endif

namespace leafra {

// Type aliases for SQLite - use system types when available
#ifdef LEAFRA_USE_SYSTEM_SQLITE_HEADERS
    // Use system SQLite types directly
    using sqlite3 = ::sqlite3;
    using sqlite3_stmt = ::sqlite3_stmt;
#else
    // Forward declarations for custom SQLite (Android/Windows)
    struct sqlite3;
    struct sqlite3_stmt;
#endif

/**
 * @brief SQLite database connection wrapper
 * 
 * Provides a C++ interface for SQLite database operations with
 * automatic resource management and error handling.
 */
class SQLiteDatabase {
public:
    /**
     * @brief Database open flags for controlling SQLite database access modes
     * 
     * These flags control how the SQLite database is opened and what operations
     * are permitted. Flags can be combined using bitwise OR operations.
     * 
     * @details
     * **Flag Combinations:**
     * - Most common: `ReadWrite | Create` (default) - Open for R/W, create if needed
     * - Read-only access: `ReadOnly` - Safe for concurrent read access
     * - Temporary database: `ReadWrite | Create | Memory` - In-memory database
     * - URI mode: `ReadWrite | URI` - Interpret filename as SQLite URI
     * 
     * **Security Notes:**
     * - ReadOnly mode prevents accidental data modification
     * - Create flag allows automatic database file creation
     * - Memory databases exist only in RAM (no file persistence)
     * - URI mode enables advanced SQLite connection parameters
     * 
     * **Example Usage:**
     * ```cpp
     * // Read-write with auto-create (default)
     * db.open("data.db", static_cast<int>(OpenFlags::ReadWrite | OpenFlags::Create));
     * 
     * // Read-only access
     * db.open("readonly.db", static_cast<int>(OpenFlags::ReadOnly));
     * 
     * // In-memory database
     * db.open(":memory:", static_cast<int>(OpenFlags::ReadWrite | OpenFlags::Memory));
     * ```
     */
    enum class OpenFlags {
        /**
         * @brief Open database for reading only
         * 
         * - Database file must already exist
         * - No write operations permitted (INSERT, UPDATE, DELETE)
         * - Safe for concurrent read access from multiple processes
         * - Ideal for read-only applications or data analysis
         * - Corresponds to SQLite SQLITE_OPEN_READONLY flag
         */
        ReadOnly = 0x00000001,
        
        /**
         * @brief Open database for reading and writing
         * 
         * - Database file must already exist (unless Create flag also set)
         * - All SQL operations permitted (SELECT, INSERT, UPDATE, DELETE)
         * - Exclusive write access (SQLite handles concurrent access)
         * - Most common flag for general database operations
         * - Corresponds to SQLite SQLITE_OPEN_READWRITE flag
         */
        ReadWrite = 0x00000002,
        
        /**
         * @brief Create database file if it doesn't exist
         * 
         * - Must be combined with ReadWrite flag
         * - Creates new database file if path doesn't exist
         * - No effect if database file already exists
         * - Essential for new database creation workflows
         * - Corresponds to SQLite SQLITE_OPEN_CREATE flag
         */
        Create = 0x00000004,
        
        /**
         * @brief Open as in-memory database
         * 
         * - Database exists only in RAM, not on disk
         * - Extremely fast access (no disk I/O)
         * - Data lost when database connection closes
         * - Useful for temporary data processing or caching
         * - Use ":memory:" as filename for pure in-memory database
         * - Corresponds to SQLite SQLITE_OPEN_MEMORY flag
         */
        Memory = 0x00000080,
        
        /**
         * @brief Interpret filename as SQLite URI
         * 
         * - Enables advanced SQLite URI parameters
         * - Allows query parameters in database filename
         * - Supports cache sharing, WAL mode, etc. via URI
         * - Example: "file:data.db?cache=shared&mode=rwc"
         * - Advanced feature for specialized use cases
         * - Corresponds to SQLite SQLITE_OPEN_URI flag
         */
        URI = 0x00000040
    };

    /**
     * @brief Column data types
     */
    enum class ColumnType {
        Integer = 1,
        Float = 2,
        Text = 3,
        Blob = 4,
        Null = 5
    };

    /**
     * @brief Query result row
     */
    class Row {
    public:
        Row(sqlite3_stmt* stmt);
        
        // Column access by index
        int getInt(int columnIndex) const;
        long long getInt64(int columnIndex) const;
        double getDouble(int columnIndex) const;
        std::string getText(int columnIndex) const;
        std::vector<uint8_t> getBlob(int columnIndex) const;
        bool isNull(int columnIndex) const;
        ColumnType getColumnType(int columnIndex) const;
        
        // Column access by name
        int getInt(const std::string& columnName) const;
        long long getInt64(const std::string& columnName) const;
        double getDouble(const std::string& columnName) const;
        std::string getText(const std::string& columnName) const;
        std::vector<uint8_t> getBlob(const std::string& columnName) const;
        bool isNull(const std::string& columnName) const;
        
        // Metadata
        int getColumnCount() const;
        std::string getColumnName(int columnIndex) const;
        int getColumnIndex(const std::string& columnName) const;
        
    private:
        sqlite3_stmt* stmt_;
        mutable std::map<std::string, int> columnIndexCache_;
    };

    /**
     * @brief Prepared statement wrapper
     */
    class Statement {
    public:
        Statement(sqlite3* db, const std::string& sql);
        ~Statement();
        
        // Non-copyable but movable
        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;
        Statement(Statement&& other) noexcept;
        Statement& operator=(Statement&& other) noexcept;
        
        // Parameter binding
        bool bindInt(int paramIndex, int value);
        bool bindInt64(int paramIndex, long long value);
        bool bindDouble(int paramIndex, double value);
        bool bindText(int paramIndex, const std::string& value);
        bool bindBlob(int paramIndex, const std::vector<uint8_t>& value);
        bool bindNull(int paramIndex);
        
        // Named parameter binding
        bool bindInt(const std::string& paramName, int value);
        bool bindInt64(const std::string& paramName, long long value);
        bool bindDouble(const std::string& paramName, double value);
        bool bindText(const std::string& paramName, const std::string& value);
        bool bindBlob(const std::string& paramName, const std::vector<uint8_t>& value);
        bool bindNull(const std::string& paramName);
        
        // Execution
        bool step();
        bool execute();  // For INSERT/UPDATE/DELETE statements
        bool reset();
        Row getCurrentRow() const;
        
        // Metadata
        int getParameterCount() const;
        int getParameterIndex(const std::string& paramName) const;
        bool isValid() const;
        
    private:
        sqlite3_stmt* stmt_;
        bool valid_;
    };

public:
    SQLiteDatabase();
    ~SQLiteDatabase();
    
    // Non-copyable but movable
    SQLiteDatabase(const SQLiteDatabase&) = delete;
    SQLiteDatabase& operator=(const SQLiteDatabase&) = delete;
    SQLiteDatabase(SQLiteDatabase&& other) noexcept;
    SQLiteDatabase& operator=(SQLiteDatabase&& other) noexcept;
    
    // Database operations
    /**
     * @brief Opens an existing SQLite database file with secure path handling
     * 
     * This method opens an existing SQLite database file using the LeafraFileManager
     * for secure path resolution. The database file must already exist and be accessible.
     * 
     * @param relative_path Relative path to the database file (e.g., "documents.db", "data/mydb.db")
     *                      - MUST be a relative path (absolute paths are rejected for security)
     *                      - Path traversal sequences ("..") are not allowed
     *                      - Will be converted to absolute path using LeafraFileManager
     *                      - Database will be opened from AppStorage directory
     * @param flags SQLite open flags (default: ReadWrite | Create)
     *              - OpenFlags::ReadOnly: Open for reading only
     *              - OpenFlags::ReadWrite: Open for reading and writing
     *              - OpenFlags::Create: Create file if it doesn't exist
     *              - OpenFlags::Memory: Open as in-memory database
     *              - OpenFlags::URI: Interpret filename as URI
     * 
     * @return true if database was opened successfully
     * @return false if opening failed due to:
     *         - Invalid path (empty, absolute, or contains "..")
     *         - File doesn't exist (when Create flag not set)
     *         - File system permission errors
     *         - SQLite errors during opening
     * 
     * @details
     * **Security Features:**
     * - Path validation prevents directory traversal attacks
     * - Only relative paths accepted (converted to absolute via FileManager)
     * - Database opened from secure AppStorage location
     * 
     * **Usage Patterns:**
     * - Use after `createdb()` to open a newly created database
     * - Can be used to open existing databases with different flags
     * - Supports all standard SQLite open modes
     * 
     * **Example Usage:**
     * ```cpp
     * SQLiteDatabase db;
     * 
     * // Open existing database for read/write
     * if (db.open("documents.db")) {
     *     LEAFRA_INFO() << "Database opened successfully";
     * }
     * 
     * // Open with specific flags (read-only)
     * if (db.open("readonly.db", static_cast<int>(OpenFlags::ReadOnly))) {
     *     LEAFRA_INFO() << "Read-only database opened";
     * }
     * 
     * // This will FAIL - absolute path not allowed
     * if (!db.open("/tmp/database.db")) {
     *     LEAFRA_ERROR() << "Absolute paths are rejected";
     * }
     * ```
     * 
     * @note This method integrates with LeafraFileManager for cross-platform path handling
     * @see LeafraFileManager::getFullPath() for path conversion details
     * @see StorageType::AppStorage for storage location information
     * @see createdb() for creating new databases with RAG schema
     */
    bool open(const std::string& relative_path, int flags = static_cast<int>(OpenFlags::ReadWrite) | static_cast<int>(OpenFlags::Create));
    bool openMemory();
    void close();
    bool isOpen() const;
    
    // SQL execution
    bool execute(const std::string& sql);
    bool execute(const std::string& sql, const std::function<bool(const Row&)>& rowCallback);
    
    // Prepared statements
    std::unique_ptr<Statement> prepare(const std::string& sql);
    
    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isInTransaction() const;
    
    // Database information
    int getLastInsertRowId() const;
    int getChanges() const;
    int getTotalChanges() const;
    std::string getVersion() const;
    
    // Error handling
    int getLastErrorCode() const;
    std::string getLastErrorMessage() const;
    
    // Utility functions
    static std::string escapeString(const std::string& str);
    static bool fileExists(const std::string& path);
    
         /**
      * @brief Creates a new SQLite database with RAG (Retrieval-Augmented Generation) schema
      * 
      * This method creates a new SQLite database file with predefined tables optimized for
      * document storage and retrieval operations. The database is created in the application's
      * storage directory using the LeafraFileManager for secure path handling.
      * 
      * @param relative_path Relative path for the database file (e.g., "documents.db", "data/mydb.db")
      *                      - MUST be a relative path (absolute paths are rejected for security)
      *                      - Path traversal sequences ("..") are not allowed
      *                      - Will be converted to absolute path using LeafraFileManager
      *                      - Database will be created in AppStorage directory
      * 
      * @return true if database and tables were created successfully (or already exist)
      * @return false if creation failed due to:
      *         - Invalid path (empty, absolute, or contains "..")
      *         - File system errors
      *         - SQLite errors during database/table creation
      * 
      * @details
      * **Created Tables:**
      * - `docs`: Document metadata (id, filename, url, creation_date, size)
      * - `chunks`: Text chunks with embeddings (id, doc_id, chunk_no, chunk_size, chunk_text, chunk_embedding)
      * - Includes foreign key constraints and performance indexes
      * 
      * **Security Features:**
      * - Path validation prevents directory traversal attacks
      * - Only relative paths accepted (converted to absolute via FileManager)
      * - Database created in secure AppStorage location
      * 
      * **Error Handling:**
      * - Atomic operation: if table creation fails, database file is deleted
      * - Comprehensive logging for debugging
      * - Returns false on any validation or creation error
      * 
      * **Example Usage:**
      * ```cpp
      * // Create database in AppStorage/documents.db
      * if (SQLiteDatabase::createdb("documents.db")) {
      *     LEAFRA_INFO() << "Database created successfully";
      * }
      * 
      * // Create database in subdirectory AppStorage/data/embeddings.db
      * if (SQLiteDatabase::createdb("data/embeddings.db")) {
      *     LEAFRA_INFO() << "Database with subdirectory created";
      * }
      * 
      * // This will FAIL - absolute path not allowed
      * if (!SQLiteDatabase::createdb("/tmp/database.db")) {
      *     LEAFRA_ERROR() << "Absolute paths are rejected";
      * }
      * ```
      * 
      * @note This method integrates with LeafraFileManager for cross-platform path handling
      * @see LeafraFileManager::getFullPath() for path conversion details
      * @see StorageType::AppStorage for storage location information
      */
     static bool createdb(const std::string& relative_path);
    
private:
    sqlite3* db_;
    bool isOpen_;
    
    void cleanup();
    bool createRAGTables();
};

/**
 * @brief RAII transaction helper
 */
class SQLiteTransaction {
public:
    explicit SQLiteTransaction(SQLiteDatabase& db);
    ~SQLiteTransaction();
    
    // Non-copyable and non-movable
    SQLiteTransaction(const SQLiteTransaction&) = delete;
    SQLiteTransaction& operator=(const SQLiteTransaction&) = delete;
    SQLiteTransaction(SQLiteTransaction&&) = delete;
    SQLiteTransaction& operator=(SQLiteTransaction&&) = delete;
    
    bool commit();
    void rollback();
    bool isActive() const;
    
private:
    SQLiteDatabase& db_;
    bool committed_;
    bool active_;
};

} // namespace leafra

#endif // __cplusplus

#endif // LEAFRA_SQLITE_H 