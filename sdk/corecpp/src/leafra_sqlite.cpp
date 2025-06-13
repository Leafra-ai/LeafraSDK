#include "leafra/leafra_sqlite.h"
#include "leafra/logger.h"
#include "leafra/leafra_filemanager.h"

#ifdef LEAFRA_HAS_SQLITE
    #ifdef LEAFRA_USE_SYSTEM_SQLITE_HEADERS
        // Use system SQLite headers (iOS/macOS/Linux)
        #include <sqlite3.h>
    #else
        // Use our custom SQLite headers (Android/Windows)
        #include "sqlite3.h"
    #endif
#endif

#include <filesystem>
#include <sstream>
#include <algorithm>

namespace leafra {

#ifdef LEAFRA_HAS_SQLITE

// ==============================================================================
// SQLiteDatabase::Row Implementation
// ==============================================================================

SQLiteDatabase::Row::Row(sqlite3_stmt* stmt) : stmt_(stmt) {
    LEAFRA_DEBUG() << "Row created for statement";
}

int SQLiteDatabase::Row::getInt(int columnIndex) const {
    return sqlite3_column_int(stmt_, columnIndex);
}

long long SQLiteDatabase::Row::getInt64(int columnIndex) const {
    return sqlite3_column_int64(stmt_, columnIndex);
}

double SQLiteDatabase::Row::getDouble(int columnIndex) const {
    return sqlite3_column_double(stmt_, columnIndex);
}

std::string SQLiteDatabase::Row::getText(int columnIndex) const {
    const unsigned char* text = sqlite3_column_text(stmt_, columnIndex);
    return text ? std::string(reinterpret_cast<const char*>(text)) : "";
}

std::vector<uint8_t> SQLiteDatabase::Row::getBlob(int columnIndex) const {
    const void* blob = sqlite3_column_blob(stmt_, columnIndex);
    int size = sqlite3_column_bytes(stmt_, columnIndex);
    
    if (blob && size > 0) {
        const uint8_t* data = static_cast<const uint8_t*>(blob);
        return std::vector<uint8_t>(data, data + size);
    }
    
    return std::vector<uint8_t>();
}

bool SQLiteDatabase::Row::isNull(int columnIndex) const {
    return sqlite3_column_type(stmt_, columnIndex) == SQLITE_NULL;
}

SQLiteDatabase::ColumnType SQLiteDatabase::Row::getColumnType(int columnIndex) const {
    int type = sqlite3_column_type(stmt_, columnIndex);
    return static_cast<ColumnType>(type);
}

int SQLiteDatabase::Row::getInt(const std::string& columnName) const {
    return getInt(getColumnIndex(columnName));
}

long long SQLiteDatabase::Row::getInt64(const std::string& columnName) const {
    return getInt64(getColumnIndex(columnName));
}

double SQLiteDatabase::Row::getDouble(const std::string& columnName) const {
    return getDouble(getColumnIndex(columnName));
}

std::string SQLiteDatabase::Row::getText(const std::string& columnName) const {
    return getText(getColumnIndex(columnName));
}

std::vector<uint8_t> SQLiteDatabase::Row::getBlob(const std::string& columnName) const {
    return getBlob(getColumnIndex(columnName));
}

bool SQLiteDatabase::Row::isNull(const std::string& columnName) const {
    return isNull(getColumnIndex(columnName));
}

int SQLiteDatabase::Row::getColumnCount() const {
    return sqlite3_column_count(stmt_);
}

std::string SQLiteDatabase::Row::getColumnName(int columnIndex) const {
    const char* name = sqlite3_column_name(stmt_, columnIndex);
    return name ? std::string(name) : "";
}

int SQLiteDatabase::Row::getColumnIndex(const std::string& columnName) const {
    auto it = columnIndexCache_.find(columnName);
    if (it != columnIndexCache_.end()) {
        return it->second;
    }
    
    int count = getColumnCount();
    for (int i = 0; i < count; ++i) {
        if (getColumnName(i) == columnName) {
            columnIndexCache_[columnName] = i;
            return i;
        }
    }
    
    LEAFRA_WARNING() << "Column not found: " << columnName;
    return -1;
}

// ==============================================================================
// SQLiteDatabase::Statement Implementation
// ==============================================================================

SQLiteDatabase::Statement::Statement(sqlite3* db, const std::string& sql) 
    : stmt_(nullptr), valid_(false) {
    
    const char* tail = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, &tail);
    
    if (result == SQLITE_OK && stmt_) {
        valid_ = true;
        LEAFRA_DEBUG() << "Statement prepared successfully: " << sql;
    } else {
        LEAFRA_ERROR() << "Failed to prepare statement: " << sql << " - " << sqlite3_errmsg(db);
        valid_ = false;
    }
}

SQLiteDatabase::Statement::~Statement() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
}

SQLiteDatabase::Statement::Statement(Statement&& other) noexcept 
    : stmt_(other.stmt_), valid_(other.valid_) {
    other.stmt_ = nullptr;
    other.valid_ = false;
}

SQLiteDatabase::Statement& SQLiteDatabase::Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
        stmt_ = other.stmt_;
        valid_ = other.valid_;
        other.stmt_ = nullptr;
        other.valid_ = false;
    }
    return *this;
}

bool SQLiteDatabase::Statement::bindInt(int paramIndex, int value) {
    if (!valid_) return false;
    return sqlite3_bind_int(stmt_, paramIndex, value) == SQLITE_OK;
}

bool SQLiteDatabase::Statement::bindInt64(int paramIndex, long long value) {
    if (!valid_) return false;
    return sqlite3_bind_int64(stmt_, paramIndex, value) == SQLITE_OK;
}

bool SQLiteDatabase::Statement::bindDouble(int paramIndex, double value) {
    if (!valid_) return false;
    return sqlite3_bind_double(stmt_, paramIndex, value) == SQLITE_OK;
}

bool SQLiteDatabase::Statement::bindText(int paramIndex, const std::string& value) {
    if (!valid_) return false;
    return sqlite3_bind_text(stmt_, paramIndex, value.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;
}

bool SQLiteDatabase::Statement::bindBlob(int paramIndex, const std::vector<uint8_t>& value) {
    if (!valid_) return false;
    return sqlite3_bind_blob(stmt_, paramIndex, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT) == SQLITE_OK;
}

bool SQLiteDatabase::Statement::bindNull(int paramIndex) {
    if (!valid_) return false;
    return sqlite3_bind_null(stmt_, paramIndex) == SQLITE_OK;
}

bool SQLiteDatabase::Statement::bindInt(const std::string& paramName, int value) {
    int index = getParameterIndex(paramName);
    return index > 0 ? bindInt(index, value) : false;
}

bool SQLiteDatabase::Statement::bindInt64(const std::string& paramName, long long value) {
    int index = getParameterIndex(paramName);
    return index > 0 ? bindInt64(index, value) : false;
}

bool SQLiteDatabase::Statement::bindDouble(const std::string& paramName, double value) {
    int index = getParameterIndex(paramName);
    return index > 0 ? bindDouble(index, value) : false;
}

bool SQLiteDatabase::Statement::bindText(const std::string& paramName, const std::string& value) {
    int index = getParameterIndex(paramName);
    return index > 0 ? bindText(index, value) : false;
}

bool SQLiteDatabase::Statement::bindBlob(const std::string& paramName, const std::vector<uint8_t>& value) {
    int index = getParameterIndex(paramName);
    return index > 0 ? bindBlob(index, value) : false;
}

bool SQLiteDatabase::Statement::bindNull(const std::string& paramName) {
    int index = getParameterIndex(paramName);
    return index > 0 ? bindNull(index) : false;
}

bool SQLiteDatabase::Statement::step() {
    if (!valid_) return false;
    int result = sqlite3_step(stmt_);
    return result == SQLITE_ROW;  // Only return true if there's actual row data
}

bool SQLiteDatabase::Statement::reset() {
    if (!valid_) return false;
    return sqlite3_reset(stmt_) == SQLITE_OK;
}

SQLiteDatabase::Row SQLiteDatabase::Statement::getCurrentRow() const {
    return Row(stmt_);
}

int SQLiteDatabase::Statement::getParameterCount() const {
    if (!valid_) return 0;
    return sqlite3_bind_parameter_count(stmt_);
}

int SQLiteDatabase::Statement::getParameterIndex(const std::string& paramName) const {
    if (!valid_) return 0;
    return sqlite3_bind_parameter_index(stmt_, paramName.c_str());
}

bool SQLiteDatabase::Statement::isValid() const {
    return valid_;
}

// ==============================================================================
// SQLiteDatabase Implementation
// ==============================================================================

SQLiteDatabase::SQLiteDatabase() : db_(nullptr), isOpen_(false) {
    LEAFRA_DEBUG() << "SQLiteDatabase created";
}

SQLiteDatabase::~SQLiteDatabase() {
    close();
    LEAFRA_DEBUG() << "SQLiteDatabase destroyed";
}

SQLiteDatabase::SQLiteDatabase(SQLiteDatabase&& other) noexcept 
    : db_(other.db_), isOpen_(other.isOpen_) {
    other.db_ = nullptr;
    other.isOpen_ = false;
}

SQLiteDatabase& SQLiteDatabase::operator=(SQLiteDatabase&& other) noexcept {
    if (this != &other) {
        close();
        db_ = other.db_;
        isOpen_ = other.isOpen_;
        other.db_ = nullptr;
        other.isOpen_ = false;
    }
    return *this;
}

bool SQLiteDatabase::open(const std::string& relative_path, int flags) {
    if (isOpen_) {
        LEAFRA_WARNING() << "Database already open";
        return true;
    }
    
    LEAFRA_INFO() << "Opening SQLite database: " << relative_path;
    
    // Validate that the path is relative (not absolute)
    if (relative_path.empty()) {
        LEAFRA_ERROR() << "Database path cannot be empty";
        return false;
    }
    
    // Check for absolute path indicators
    if (relative_path[0] == '/' || relative_path[0] == '\\' || 
        (relative_path.length() > 1 && relative_path[1] == ':')) { // Windows drive letter
        LEAFRA_ERROR() << "Database path must be relative, not absolute: " << relative_path;
        return false;
    }
    
    // Check for path traversal attempts
    if (relative_path.find("..") != std::string::npos) {
        LEAFRA_ERROR() << "Database path cannot contain path traversal sequences (..): " << relative_path;
        return false;
    }
    
    // Convert relative path to absolute path using file manager
    std::string absolutePath = FileManager::getAbsolutePath(StorageType::AppStorage, relative_path);
    if (absolutePath.empty()) {
        LEAFRA_ERROR() << "Failed to convert relative path to absolute path: " << relative_path;
        return false;
    }
    
    LEAFRA_DEBUG() << "Converted relative path '" << relative_path << "' to absolute path: " << absolutePath;
    
    int result = sqlite3_open_v2(absolutePath.c_str(), &db_, flags, nullptr);
    
    if (result == SQLITE_OK) {
        isOpen_ = true;
        LEAFRA_INFO() << "SQLite database opened successfully: " << absolutePath;
        return true;
    } else {
        LEAFRA_ERROR() << "Failed to open SQLite database: " << absolutePath 
                      << " Error: " << sqlite3_errmsg(db_);
        cleanup();
        return false;
    }
}

bool SQLiteDatabase::openMemory() {
    return open(":memory:", static_cast<int>(OpenFlags::ReadWrite) | static_cast<int>(OpenFlags::Create));
}

void SQLiteDatabase::close() {
    if (isOpen_ && db_) {
        LEAFRA_INFO() << "Closing SQLite database";
        sqlite3_close_v2(db_);
        cleanup();
    }
}

bool SQLiteDatabase::isOpen() const {
    return isOpen_;
}

bool SQLiteDatabase::execute(const std::string& sql) {
    if (!isOpen_) {
        LEAFRA_ERROR() << "Database not open";
        return false;
    }
    
    LEAFRA_DEBUG() << "Executing SQL: " << sql;
    
    char* errorMsg = nullptr;
    int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMsg);
    
    if (result == SQLITE_OK) {
        LEAFRA_DEBUG() << "SQL executed successfully";
        return true;
    } else {
        LEAFRA_ERROR() << "SQL execution failed: " << (errorMsg ? errorMsg : "Unknown error");
        if (errorMsg) {
            sqlite3_free(errorMsg);
        }
        return false;
    }
}

bool SQLiteDatabase::execute(const std::string& sql, const std::function<bool(const Row&)>& rowCallback) {
    auto stmt = prepare(sql);
    if (!stmt || !stmt->isValid()) {
        return false;
    }
    
    while (stmt->step()) {
        Row row = stmt->getCurrentRow();
        if (!rowCallback(row)) {
            break; // Callback requested to stop
        }
    }
    
    return true;
}

std::unique_ptr<SQLiteDatabase::Statement> SQLiteDatabase::prepare(const std::string& sql) {
    if (!isOpen_) {
        LEAFRA_ERROR() << "Database not open";
        return nullptr;
    }
    
    auto stmt = std::make_unique<Statement>(db_, sql);
    return stmt->isValid() ? std::move(stmt) : nullptr;
}

bool SQLiteDatabase::beginTransaction() {
    return execute("BEGIN TRANSACTION");
}

bool SQLiteDatabase::commitTransaction() {
    return execute("COMMIT");
}

bool SQLiteDatabase::rollbackTransaction() {
    return execute("ROLLBACK");
}

bool SQLiteDatabase::isInTransaction() const {
    if (!isOpen_) return false;
    return !sqlite3_get_autocommit(db_);
}

int SQLiteDatabase::getLastInsertRowId() const {
    if (!isOpen_) return 0;
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

int SQLiteDatabase::getChanges() const {
    if (!isOpen_) return 0;
    return sqlite3_changes(db_);
}

int SQLiteDatabase::getTotalChanges() const {
    if (!isOpen_) return 0;
    return sqlite3_total_changes(db_);
}

std::string SQLiteDatabase::getVersion() const {
    return sqlite3_libversion();
}

int SQLiteDatabase::getLastErrorCode() const {
    if (!isOpen_) return SQLITE_MISUSE;
    return sqlite3_errcode(db_);
}

std::string SQLiteDatabase::getLastErrorMessage() const {
    if (!isOpen_) return "Database not open";
    return sqlite3_errmsg(db_);
}

std::string SQLiteDatabase::escapeString(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.length() * 2);
    
    for (char c : str) {
        if (c == '\'') {
            escaped += "''";
        } else {
            escaped += c;
        }
    }
    
    return escaped;
}

bool SQLiteDatabase::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

bool SQLiteDatabase::createdb(const std::string& relative_path) {
    LEAFRA_DEBUG() << "Creating database: " << relative_path;
    
    // Validate that the path is relative (not absolute)
    if (relative_path.empty()) {
        LEAFRA_ERROR() << "Database path cannot be empty";
        return false;
    }
    
    // Check for absolute path indicators
    if (relative_path[0] == '/' || relative_path[0] == '\\' || 
        (relative_path.length() > 1 && relative_path[1] == ':')) { // Windows drive letter
        LEAFRA_ERROR() << "Database path must be relative, not absolute: " << relative_path;
        return false;
    }
    
    // Check for path traversal attempts
    if (relative_path.find("..") != std::string::npos) {
        LEAFRA_ERROR() << "Database path cannot contain path traversal sequences (..): " << relative_path;
        return false;
    }
    
    // Convert relative path to absolute path using file manager
    std::string absolutePath = FileManager::getAbsolutePath(StorageType::AppStorage, relative_path);
    if (absolutePath.empty()) {
        LEAFRA_ERROR() << "Failed to convert relative path to absolute path: " << relative_path;
        return false;
    }
    
    LEAFRA_DEBUG() << "Converted relative path '" << relative_path << "' to absolute path: " << absolutePath;
    
    // Create parent directories if they don't exist
    std::filesystem::path dbPath(absolutePath);
    std::filesystem::path parentDir = dbPath.parent_path();
    if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
        std::error_code ec;
        if (!std::filesystem::create_directories(parentDir, ec)) {
            LEAFRA_ERROR() << "Failed to create parent directories for: " << absolutePath 
                          << " Error: " << ec.message();
            return false;
        }
        LEAFRA_DEBUG() << "Created parent directories: " << parentDir;
    }
    
    // Check if file already exists
    if (fileExists(absolutePath)) {
        LEAFRA_WARNING() << "Database file already exists: " << absolutePath;
        return true; // Consider existing file as success
    }
    
    // Create the database by opening it with CREATE flag
    sqlite3* db = nullptr;
    
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int result = sqlite3_open_v2(absolutePath.c_str(), &db, flags, nullptr);
    
    if (result == SQLITE_OK) {
        LEAFRA_DEBUG() << "Database created successfully: " << absolutePath;
        
        // Use the already opened database handle to create RAG tables
        SQLiteDatabase newDb;
        newDb.db_ = db;
        newDb.isOpen_ = true;
        
        bool tablesCreated = newDb.createRAGTables();
        
        // Clean up - the newDb destructor will close the database
        newDb.db_ = nullptr;
        newDb.isOpen_ = false;
        sqlite3_close(db);
        
        if (tablesCreated) {
            LEAFRA_DEBUG() << "Database and RAG tables created successfully: " << absolutePath;
            return true;
        } else {
            LEAFRA_ERROR() << "Database created but failed to create RAG tables: " << absolutePath;
            // Clean up by deleting the partially created database file
            if (std::filesystem::remove(absolutePath)) {
                LEAFRA_DEBUG() << "Cleaned up partially created database file: " << absolutePath;
            } else {
                LEAFRA_WARNING() << "Failed to clean up partially created database file: " << absolutePath;
            }
            return false;
        }
    } else {
        LEAFRA_ERROR() << "Failed to create database: " << absolutePath 
                      << " Error: " << sqlite3_errmsg(db);
        if (db) {
            sqlite3_close(db);
        }
        // Clean up any partially created file
        if (fileExists(absolutePath)) {
            if (std::filesystem::remove(absolutePath)) {
                LEAFRA_DEBUG() << "Cleaned up partially created database file: " << absolutePath;
            } else {
                LEAFRA_WARNING() << "Failed to clean up partially created database file: " << absolutePath;
            }
        }
        return false;
    }
}






void SQLiteDatabase::cleanup() {
    db_ = nullptr;
    isOpen_ = false;
}

bool SQLiteDatabase::createRAGTables() {
    if (!isOpen_) {
        LEAFRA_ERROR() << "Database not open";
        return false;
    }
    
    LEAFRA_DEBUG() << "Creating RAG tables (docs and chunks)";
    
    // Start a transaction for atomic table creation
    SQLiteTransaction transaction(*this);
    
    // Create docs table
    const std::string createDocsTable = R"(
        CREATE TABLE IF NOT EXISTS docs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            filename TEXT NOT NULL,
            url TEXT,
            creation_date DATETIME DEFAULT CURRENT_TIMESTAMP,
            size INTEGER NOT NULL
        )
    )";
    
    if (!execute(createDocsTable)) {
        LEAFRA_ERROR() << "Failed to create docs table";
        return false;
    }
    
    // Create chunks table with foreign key to docs table
    const std::string createChunksTable = R"(
        CREATE TABLE IF NOT EXISTS chunks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            doc_id INTEGER NOT NULL,
            chunk_page_number INTEGER NOT NULL,
            chunk_no INTEGER NOT NULL,
            chunk_size INTEGER NOT NULL,
            chunk_text TEXT NOT NULL,
            chunk_embedding BLOB,
            FOREIGN KEY (doc_id) REFERENCES docs(id) ON DELETE CASCADE
        )
    )";
    
    if (!execute(createChunksTable)) {
        LEAFRA_ERROR() << "Failed to create chunks table";
        return false;
    }
    
    // Create indexes for better performance
    const std::string createDocsFilenameIndex = "CREATE INDEX IF NOT EXISTS idx_docs_filename ON docs(filename)";
    const std::string createChunksDocIdIndex = "CREATE INDEX IF NOT EXISTS idx_chunks_doc_id ON chunks(doc_id)";
    const std::string createChunksChunkNoIndex = "CREATE INDEX IF NOT EXISTS idx_chunks_chunk_no ON chunks(doc_id, chunk_no)";
    
    if (!execute(createDocsFilenameIndex) || 
        !execute(createChunksDocIdIndex) || 
        !execute(createChunksChunkNoIndex)) {
        LEAFRA_ERROR() << "Failed to create indexes";
        return false;
    }
    
    // Commit the transaction to ensure tables are written to disk
    if (!transaction.commit()) {
        LEAFRA_ERROR() << "Failed to commit RAG tables creation transaction";
        return false;
    }
    
    LEAFRA_DEBUG() << "RAG tables created successfully";
    return true;
}

// ==============================================================================
// SQLiteTransaction Implementation
// ==============================================================================

SQLiteTransaction::SQLiteTransaction(SQLiteDatabase& db) 
    : db_(db), committed_(false), active_(false) {
    
    if (db_.beginTransaction()) {
        active_ = true;
        LEAFRA_DEBUG() << "Transaction started";
    } else {
        LEAFRA_ERROR() << "Failed to start transaction";
    }
}

SQLiteTransaction::~SQLiteTransaction() {
    if (active_ && !committed_) {
        rollback();
    }
}

bool SQLiteTransaction::commit() {
    if (!active_ || committed_) {
        return false;
    }
    
    if (db_.commitTransaction()) {
        committed_ = true;
        active_ = false;
        LEAFRA_DEBUG() << "Transaction committed";
        return true;
    } else {
        LEAFRA_ERROR() << "Failed to commit transaction";
        return false;
    }
}

void SQLiteTransaction::rollback() {
    if (!active_) {
        return;
    }
    
    db_.rollbackTransaction();
    active_ = false;
    LEAFRA_DEBUG() << "Transaction rolled back";
}

bool SQLiteTransaction::isActive() const {
    return active_;
}

#else // !LEAFRA_HAS_SQLITE

// ==============================================================================
// Stub implementations when SQLite is not available
// ==============================================================================

SQLiteDatabase::Row::Row(sqlite3_stmt* stmt) : stmt_(nullptr) {}
int SQLiteDatabase::Row::getInt(int columnIndex) const { return 0; }
long long SQLiteDatabase::Row::getInt64(int columnIndex) const { return 0; }
double SQLiteDatabase::Row::getDouble(int columnIndex) const { return 0.0; }
std::string SQLiteDatabase::Row::getText(int columnIndex) const { return ""; }
std::vector<uint8_t> SQLiteDatabase::Row::getBlob(int columnIndex) const { return {}; }
bool SQLiteDatabase::Row::isNull(int columnIndex) const { return true; }
SQLiteDatabase::ColumnType SQLiteDatabase::Row::getColumnType(int columnIndex) const { return ColumnType::Null; }
int SQLiteDatabase::Row::getInt(const std::string& columnName) const { return 0; }
long long SQLiteDatabase::Row::getInt64(const std::string& columnName) const { return 0; }
double SQLiteDatabase::Row::getDouble(const std::string& columnName) const { return 0.0; }
std::string SQLiteDatabase::Row::getText(const std::string& columnName) const { return ""; }
std::vector<uint8_t> SQLiteDatabase::Row::getBlob(const std::string& columnName) const { return {}; }
bool SQLiteDatabase::Row::isNull(const std::string& columnName) const { return true; }
int SQLiteDatabase::Row::getColumnCount() const { return 0; }
std::string SQLiteDatabase::Row::getColumnName(int columnIndex) const { return ""; }
int SQLiteDatabase::Row::getColumnIndex(const std::string& columnName) const { return -1; }

SQLiteDatabase::Statement::Statement(sqlite3* db, const std::string& sql) : stmt_(nullptr), valid_(false) {}
SQLiteDatabase::Statement::~Statement() {}
SQLiteDatabase::Statement::Statement(Statement&& other) noexcept : stmt_(nullptr), valid_(false) {}
SQLiteDatabase::Statement& SQLiteDatabase::Statement::operator=(Statement&& other) noexcept { return *this; }
bool SQLiteDatabase::Statement::bindInt(int paramIndex, int value) { return false; }
bool SQLiteDatabase::Statement::bindInt64(int paramIndex, long long value) { return false; }
bool SQLiteDatabase::Statement::bindDouble(int paramIndex, double value) { return false; }
bool SQLiteDatabase::Statement::bindText(int paramIndex, const std::string& value) { return false; }
bool SQLiteDatabase::Statement::bindBlob(int paramIndex, const std::vector<uint8_t>& value) { return false; }
bool SQLiteDatabase::Statement::bindNull(int paramIndex) { return false; }
bool SQLiteDatabase::Statement::bindInt(const std::string& paramName, int value) { return false; }
bool SQLiteDatabase::Statement::bindInt64(const std::string& paramName, long long value) { return false; }
bool SQLiteDatabase::Statement::bindDouble(const std::string& paramName, double value) { return false; }
bool SQLiteDatabase::Statement::bindText(const std::string& paramName, const std::string& value) { return false; }
bool SQLiteDatabase::Statement::bindBlob(const std::string& paramName, const std::vector<uint8_t>& value) { return false; }
bool SQLiteDatabase::Statement::bindNull(const std::string& paramName) { return false; }
bool SQLiteDatabase::Statement::step() { return false; }
bool SQLiteDatabase::Statement::reset() { return false; }
SQLiteDatabase::Row SQLiteDatabase::Statement::getCurrentRow() const { return Row(nullptr); }
int SQLiteDatabase::Statement::getParameterCount() const { return 0; }
int SQLiteDatabase::Statement::getParameterIndex(const std::string& paramName) const { return 0; }
bool SQLiteDatabase::Statement::isValid() const { return false; }

SQLiteDatabase::SQLiteDatabase() : db_(nullptr), isOpen_(false) {
    LEAFRA_WARNING() << "SQLite not available - using stub implementation";
}
SQLiteDatabase::~SQLiteDatabase() {}
SQLiteDatabase::SQLiteDatabase(SQLiteDatabase&& other) noexcept : db_(nullptr), isOpen_(false) {}
SQLiteDatabase& SQLiteDatabase::operator=(SQLiteDatabase&& other) noexcept { return *this; }
bool SQLiteDatabase::open(const std::string& path, int flags) { 
    LEAFRA_ERROR() << "SQLite not available - cannot open database";
    return false; 
}
bool SQLiteDatabase::openMemory() { return false; }
void SQLiteDatabase::close() {}
bool SQLiteDatabase::isOpen() const { return false; }
bool SQLiteDatabase::execute(const std::string& sql) { return false; }
bool SQLiteDatabase::execute(const std::string& sql, const std::function<bool(const Row&)>& rowCallback) { return false; }
std::unique_ptr<SQLiteDatabase::Statement> SQLiteDatabase::prepare(const std::string& sql) { return nullptr; }
bool SQLiteDatabase::beginTransaction() { return false; }
bool SQLiteDatabase::commitTransaction() { return false; }
bool SQLiteDatabase::rollbackTransaction() { return false; }
bool SQLiteDatabase::isInTransaction() const { return false; }
int SQLiteDatabase::getLastInsertRowId() const { return 0; }
int SQLiteDatabase::getChanges() const { return 0; }
int SQLiteDatabase::getTotalChanges() const { return 0; }
std::string SQLiteDatabase::getVersion() const { return "SQLite not available"; }
int SQLiteDatabase::getLastErrorCode() const { return -1; }
std::string SQLiteDatabase::getLastErrorMessage() const { return "SQLite not available"; }
std::string SQLiteDatabase::escapeString(const std::string& str) { return str; }
bool SQLiteDatabase::fileExists(const std::string& path) { return std::filesystem::exists(path); }
bool SQLiteDatabase::createdb(const std::string& path) { 
    LEAFRA_ERROR() << "SQLite not available - cannot create database";
    return false; 
}
bool SQLiteDatabase::createRAGTables() { 
    LEAFRA_ERROR() << "SQLite not available - cannot create RAG tables";
    return false; 
}
void SQLiteDatabase::cleanup() {}

SQLiteTransaction::SQLiteTransaction(SQLiteDatabase& db) : db_(db), committed_(false), active_(false) {}
SQLiteTransaction::~SQLiteTransaction() {}
bool SQLiteTransaction::commit() { return false; }
void SQLiteTransaction::rollback() {}
bool SQLiteTransaction::isActive() const { return false; }

#endif // LEAFRA_HAS_SQLITE

} // namespace leafra 