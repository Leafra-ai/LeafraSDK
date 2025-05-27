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

namespace leafra {

// Forward declarations
struct sqlite3;
struct sqlite3_stmt;

/**
 * @brief SQLite database connection wrapper
 * 
 * Provides a C++ interface for SQLite database operations with
 * automatic resource management and error handling.
 */
class SQLiteDatabase {
public:
    /**
     * @brief Database open flags
     */
    enum class OpenFlags {
        ReadOnly = 0x00000001,
        ReadWrite = 0x00000002,
        Create = 0x00000004,
        Memory = 0x00000080,
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
    bool open(const std::string& path, int flags = static_cast<int>(OpenFlags::ReadWrite) | static_cast<int>(OpenFlags::Create));
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
    
private:
    sqlite3* db_;
    bool isOpen_;
    
    void cleanup();
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