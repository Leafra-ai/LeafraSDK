/*
** LeafraSDK SQLite Unit Tests - Basic Operations
** Tests for database creation, opening, closing, and basic SQL execution
*/

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>

#include "leafra/leafra_sqlite.h"
#include "leafra/leafra_filemanager.h"
#include "leafra/logger.h"

using namespace leafra;

// Test counter
static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            std::cout << "✅ PASS: " << message << std::endl; \
        } else { \
            std::cout << "❌ FAIL: " << message << std::endl; \
        } \
    } while(0)

// Helper function to clean up test databases
void cleanupTestDatabase(const std::string& relative_path) {
    std::string full_path = FileManager::getAbsolutePath(StorageType::AppStorage, relative_path);
    if (!full_path.empty() && std::filesystem::exists(full_path)) {
        std::filesystem::remove(full_path);
    }
}

void test_createdb_basic() {
    std::cout << "\n=== Testing SQLiteDatabase::createdb() Basic Functionality ===" << std::endl;
    
    // Clean up any existing test database
    cleanupTestDatabase("test_basic.db");
    
    // Test 1: Create new database with valid relative path
    bool result = SQLiteDatabase::createdb("test_basic.db");
    TEST_ASSERT(result == true, "Create new database with valid relative path");
    
    // Test 2: Verify database file was created
    std::string full_path = FileManager::getAbsolutePath(StorageType::AppStorage, "test_basic.db");
    TEST_ASSERT(!full_path.empty() && std::filesystem::exists(full_path), "Database file exists after creation");
    
    // Test 3: Create database that already exists (should succeed)
    result = SQLiteDatabase::createdb("test_basic.db");
    TEST_ASSERT(result == true, "Create database that already exists should succeed");
    
    // Test 4: Create database in subdirectory
    cleanupTestDatabase("subdir/test_subdir.db");
    result = SQLiteDatabase::createdb("subdir/test_subdir.db");
    TEST_ASSERT(result == true, "Create database in subdirectory");
    
    // Cleanup
    cleanupTestDatabase("test_basic.db");
    cleanupTestDatabase("subdir/test_subdir.db");
}

void test_createdb_validation() {
    std::cout << "\n=== Testing SQLiteDatabase::createdb() Path Validation ===" << std::endl;
    
    // Test 1: Empty path should fail
    bool result = SQLiteDatabase::createdb("");
    TEST_ASSERT(result == false, "Empty path should be rejected");
    
    // Test 2: Absolute path should fail (Unix)
    result = SQLiteDatabase::createdb("/tmp/absolute.db");
    TEST_ASSERT(result == false, "Absolute Unix path should be rejected");
    
    // Test 3: Absolute path should fail (Windows)
    result = SQLiteDatabase::createdb("C:\\temp\\absolute.db");
    TEST_ASSERT(result == false, "Absolute Windows path should be rejected");
    
    // Test 4: Path traversal should fail
    result = SQLiteDatabase::createdb("../outside.db");
    TEST_ASSERT(result == false, "Path traversal should be rejected");
    
    // Test 5: Path traversal in subdirectory should fail
    result = SQLiteDatabase::createdb("subdir/../outside.db");
    TEST_ASSERT(result == false, "Path traversal in subdirectory should be rejected");
}

void test_open_basic() {
    std::cout << "\n=== Testing SQLiteDatabase::open() Basic Functionality ===" << std::endl;
    
    // Setup: Create a test database first
    cleanupTestDatabase("test_open.db");
    bool created = SQLiteDatabase::createdb("test_open.db");
    TEST_ASSERT(created == true, "Setup: Create test database for opening");
    
    // Test 1: Open existing database
    SQLiteDatabase db;
    bool result = db.open("test_open.db");
    TEST_ASSERT(result == true, "Open existing database");
    TEST_ASSERT(db.isOpen() == true, "Database should report as open");
    
    // Test 2: Close database
    db.close();
    TEST_ASSERT(db.isOpen() == false, "Database should report as closed after close()");
    
    // Test 3: Open with ReadWrite flag
    result = db.open("test_open.db", static_cast<int>(SQLiteDatabase::OpenFlags::ReadWrite));
    TEST_ASSERT(result == true, "Open with ReadWrite flag");
    db.close();
    
    // Test 4: Open with ReadOnly flag
    result = db.open("test_open.db", static_cast<int>(SQLiteDatabase::OpenFlags::ReadOnly));
    TEST_ASSERT(result == true, "Open with ReadOnly flag");
    db.close();
    
    // Test 5: Open with Create flag (file exists)
    int readwrite_create_flags = static_cast<int>(SQLiteDatabase::OpenFlags::ReadWrite) | 
                                static_cast<int>(SQLiteDatabase::OpenFlags::Create);
    result = db.open("test_open.db", readwrite_create_flags);
    TEST_ASSERT(result == true, "Open existing file with Create flag");
    db.close();
    
    // Cleanup
    cleanupTestDatabase("test_open.db");
}

void test_open_validation() {
    std::cout << "\n=== Testing SQLiteDatabase::open() Path Validation ===" << std::endl;
    
    SQLiteDatabase db;
    
    // Test 1: Empty path should fail
    bool result = db.open("");
    TEST_ASSERT(result == false, "Empty path should be rejected");
    
    // Test 2: Absolute path should fail (Unix)
    result = db.open("/tmp/absolute.db");
    TEST_ASSERT(result == false, "Absolute Unix path should be rejected");
    
    // Test 3: Absolute path should fail (Windows)
    result = db.open("C:\\temp\\absolute.db");
    TEST_ASSERT(result == false, "Absolute Windows path should be rejected");
    
    // Test 4: Path traversal should fail
    result = db.open("../outside.db");
    TEST_ASSERT(result == false, "Path traversal should be rejected");
    
    // Test 5: Path traversal in subdirectory should fail
    result = db.open("subdir/../outside.db");
    TEST_ASSERT(result == false, "Path traversal in subdirectory should be rejected");
}

void test_basic_sql_operations() {
    std::cout << "\n=== Testing Basic SQL Operations ===" << std::endl;
    
    // Setup: Create and open test database
    cleanupTestDatabase("test_sql.db");
    bool created = SQLiteDatabase::createdb("test_sql.db");
    TEST_ASSERT(created == true, "Setup: Create test database");
    
    SQLiteDatabase db;
    bool opened = db.open("test_sql.db");
    TEST_ASSERT(opened == true, "Setup: Open test database");
    
    // Test 1: Create a simple table
    bool result = db.execute("CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT, value INTEGER)");
    TEST_ASSERT(result == true, "Create simple table");
    
    // Test 2: Insert data
    result = db.execute("INSERT INTO test_table (name, value) VALUES ('test1', 100)");
    TEST_ASSERT(result == true, "Insert data into table");
    
    // Test 3: Insert more data
    result = db.execute("INSERT INTO test_table (name, value) VALUES ('test2', 200)");
    TEST_ASSERT(result == true, "Insert second row");
    
    // Test 4: Query data with callback
    int row_count = 0;
    result = db.execute("SELECT * FROM test_table", [&row_count](const SQLiteDatabase::Row& row) {
        row_count++;
        std::string name = row.getText(1);  // name column
        int value = row.getInt(2);          // value column
        std::cout << "  Row " << row_count << ": name=" << name << ", value=" << value << std::endl;
        return true; // Continue processing
    });
    TEST_ASSERT(result == true, "Query data with callback");
    TEST_ASSERT(row_count == 2, "Should have retrieved 2 rows");
    
    // Test 5: Update data
    result = db.execute("UPDATE test_table SET value = 150 WHERE name = 'test1'");
    TEST_ASSERT(result == true, "Update data");
    
    // Test 6: Delete data
    result = db.execute("DELETE FROM test_table WHERE name = 'test2'");
    TEST_ASSERT(result == true, "Delete data");
    
    // Test 7: Verify changes
    row_count = 0;
    result = db.execute("SELECT * FROM test_table", [&row_count](const SQLiteDatabase::Row& row) {
        row_count++;
        int value = row.getInt(2);
        TEST_ASSERT(value == 150, "Updated value should be 150");
        return true;
    });
    TEST_ASSERT(row_count == 1, "Should have 1 row after delete");
    
    db.close();
    cleanupTestDatabase("test_sql.db");
}

void test_rag_tables_creation() {
    std::cout << "\n=== Testing RAG Tables Creation ===" << std::endl;
    
    // Setup: Create database (should automatically create RAG tables)
    cleanupTestDatabase("test_rag.db");
    bool created = SQLiteDatabase::createdb("test_rag.db");
    TEST_ASSERT(created == true, "Create database with RAG tables");
    
    SQLiteDatabase db;
    bool opened = db.open("test_rag.db");
    TEST_ASSERT(opened == true, "Open database with RAG tables");
    
    // Test 1: Verify docs table exists
    bool result = db.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='docs'", 
        [](const SQLiteDatabase::Row& row) {
            std::cout << "  Found table: " << row.getText(0) << std::endl;
            return true;
        });
    TEST_ASSERT(result == true, "docs table should exist");
    
    // Test 2: Verify chunks table exists
    result = db.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='chunks'", 
        [](const SQLiteDatabase::Row& row) {
            std::cout << "  Found table: " << row.getText(0) << std::endl;
            return true;
        });
    TEST_ASSERT(result == true, "chunks table should exist");
    
    // Test 3: Test inserting into docs table
    result = db.execute("INSERT INTO docs (filename, url, size) VALUES ('test.txt', 'http://example.com', 1024)");
    TEST_ASSERT(result == true, "Insert into docs table");
    
    // Test 4: Test inserting into chunks table
    result = db.execute("INSERT INTO chunks (doc_id, chunk_no, chunk_size, chunk_text) VALUES (1, 1, 100, 'This is a test chunk')");
    TEST_ASSERT(result == true, "Insert into chunks table");
    
    // Test 5: Test foreign key relationship
    int doc_count = 0;
    result = db.execute("SELECT d.filename, c.chunk_text FROM docs d JOIN chunks c ON d.id = c.doc_id", 
        [&doc_count](const SQLiteDatabase::Row& row) {
            doc_count++;
            std::cout << "  Doc: " << row.getText(0) << ", Chunk: " << row.getText(1) << std::endl;
            return true;
        });
    TEST_ASSERT(result == true && doc_count == 1, "Foreign key relationship works");
    
    db.close();
    cleanupTestDatabase("test_rag.db");
}

void test_memory_database() {
    std::cout << "\n=== Testing In-Memory Database ===" << std::endl;
    
    SQLiteDatabase db;
    
    // Test 1: Open memory database
    bool result = db.openMemory();
    TEST_ASSERT(result == true, "Open in-memory database");
    TEST_ASSERT(db.isOpen() == true, "Memory database should report as open");
    
    // Test 2: Create table in memory (use IF NOT EXISTS to handle any existing table)
    result = db.execute("CREATE TABLE IF NOT EXISTS memory_test (id INTEGER, data TEXT)");
    TEST_ASSERT(result == true, "Create table in memory database");
    
    // Clear any existing data
    db.execute("DELETE FROM memory_test");
    
    // Test 3: Insert and query data
    result = db.execute("INSERT INTO memory_test VALUES (1, 'memory data')");
    TEST_ASSERT(result == true, "Insert data into memory database");
    
    int row_count = 0;
    result = db.execute("SELECT * FROM memory_test", [&row_count](const SQLiteDatabase::Row& row) {
        row_count++;
        TEST_ASSERT(row.getInt(0) == 1, "Memory data ID should be 1");
        TEST_ASSERT(row.getText(1) == "memory data", "Memory data text should match");
        return true;
    });
    TEST_ASSERT(result == true && row_count == 1, "Query memory database");
    
    db.close();
}

int main() {
    std::cout << "🧪 LeafraSDK SQLite Unit Tests - Basic Operations" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    // Initialize logging
    LEAFRA_INFO() << "Starting SQLite basic unit tests";
    
    // Run all tests
    test_createdb_basic();
    test_createdb_validation();
    test_open_basic();
    test_open_validation();
    test_basic_sql_operations();
    test_rag_tables_creation();
    test_memory_database();
    
    // Print results
    std::cout << "\n📊 Test Results:" << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << "Total tests: " << tests_run << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << (tests_run - tests_passed) << std::endl;
    
    if (tests_passed == tests_run) {
        std::cout << "🎉 All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "💥 Some tests failed!" << std::endl;
        return 1;
    }
} 