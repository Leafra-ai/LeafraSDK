/*
** LeafraSDK SQLite Unit Tests - Advanced Operations
** Tests for prepared statements, transactions, error handling, and advanced features
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

void test_prepared_statements() {
    std::cout << "\n=== Testing Prepared Statements ===" << std::endl;
    
    // Setup: Create and open test database
    cleanupTestDatabase("test_prepared.db");
    bool created = SQLiteDatabase::createdb("test_prepared.db");
    TEST_ASSERT(created == true, "Setup: Create test database");
    
    SQLiteDatabase db;
    bool opened = db.open("test_prepared.db");
    TEST_ASSERT(opened == true, "Setup: Open test database");
    
    // Create test table
    bool result = db.execute("CREATE TABLE prepared_test (id INTEGER PRIMARY KEY, name TEXT, value REAL, data BLOB)");
    TEST_ASSERT(result == true, "Setup: Create test table");
    
    // Test 1: Prepare INSERT statement
    auto stmt = db.prepare("INSERT INTO prepared_test (name, value, data) VALUES (?, ?, ?)");
    TEST_ASSERT(stmt != nullptr, "Prepare INSERT statement");
    TEST_ASSERT(stmt->isValid() == true, "Prepared statement should be valid");
    
    // Test 2: Bind parameters and execute
    std::vector<uint8_t> test_blob{0x01, 0x02, 0x03, 0x04};
    bool bound = stmt->bindText(1, "test_name");
    TEST_ASSERT(bound == true, "Bind text parameter");
    
    bound = stmt->bindDouble(2, 3.14159);
    TEST_ASSERT(bound == true, "Bind double parameter");
    
    bound = stmt->bindBlob(3, test_blob);
    TEST_ASSERT(bound == true, "Bind blob parameter");
    
    bool stepped = stmt->step();
    TEST_ASSERT(stepped == false, "INSERT step should return false (no rows)");
    
    // Test 3: Prepare SELECT statement
    auto select_stmt = db.prepare("SELECT * FROM prepared_test WHERE name = ?");
    TEST_ASSERT(select_stmt != nullptr, "Prepare SELECT statement");
    
    select_stmt->bindText(1, "test_name");
    stepped = select_stmt->step();
    TEST_ASSERT(stepped == true, "SELECT step should return true (has row)");
    
    // Test 4: Read data from prepared statement
    auto row = select_stmt->getCurrentRow();
    TEST_ASSERT(row.getInt(0) == 1, "ID should be 1");
    TEST_ASSERT(row.getText(1) == "test_name", "Name should match");
    TEST_ASSERT(std::abs(row.getDouble(2) - 3.14159) < 0.00001, "Value should match");
    
    auto retrieved_blob = row.getBlob(3);
    TEST_ASSERT(retrieved_blob == test_blob, "Blob data should match");
    
    // Test 5: Named parameter binding
    auto named_stmt = db.prepare("INSERT INTO prepared_test (name, value) VALUES (:name, :value)");
    TEST_ASSERT(named_stmt != nullptr, "Prepare statement with named parameters");
    
    named_stmt->bindText(":name", "named_test");
    named_stmt->bindDouble(":value", 2.71828);
    stepped = named_stmt->step();
    TEST_ASSERT(stepped == false, "Named parameter INSERT should succeed");
    
    db.close();
    cleanupTestDatabase("test_prepared.db");
}

void test_transactions() {
    std::cout << "\n=== Testing Transactions ===" << std::endl;
    
    // Setup: Create and open test database
    cleanupTestDatabase("test_transactions.db");
    bool created = SQLiteDatabase::createdb("test_transactions.db");
    TEST_ASSERT(created == true, "Setup: Create test database");
    
    SQLiteDatabase db;
    bool opened = db.open("test_transactions.db");
    TEST_ASSERT(opened == true, "Setup: Open test database");
    
    // Create test table
    bool result = db.execute("CREATE TABLE transaction_test (id INTEGER PRIMARY KEY, value INTEGER)");
    TEST_ASSERT(result == true, "Setup: Create test table");
    
    // Test 1: Basic transaction commit
    {
        SQLiteTransaction transaction(db);
        TEST_ASSERT(db.isInTransaction() == true, "Should be in transaction");
        
        result = db.execute("INSERT INTO transaction_test (value) VALUES (100)");
        TEST_ASSERT(result == true, "Insert in transaction");
        
        bool committed = transaction.commit();
        TEST_ASSERT(committed == true, "Transaction commit should succeed");
        TEST_ASSERT(db.isInTransaction() == false, "Should not be in transaction after commit");
    }
    
    // Verify data was committed
    int row_count = 0;
    result = db.execute("SELECT * FROM transaction_test", [&row_count](const SQLiteDatabase::Row& row) {
        row_count++;
        TEST_ASSERT(row.getInt(1) == 100, "Committed value should be 100");
        return true;
    });
    TEST_ASSERT(row_count == 1, "Should have 1 committed row");
    
    // Test 2: Transaction rollback
    {
        SQLiteTransaction transaction(db);
        TEST_ASSERT(db.isInTransaction() == true, "Should be in transaction");
        
        result = db.execute("INSERT INTO transaction_test (value) VALUES (200)");
        TEST_ASSERT(result == true, "Insert in transaction");
        
        transaction.rollback();
        TEST_ASSERT(db.isInTransaction() == false, "Should not be in transaction after rollback");
    }
    
    // Verify data was rolled back
    row_count = 0;
    result = db.execute("SELECT * FROM transaction_test", [&row_count](const SQLiteDatabase::Row& row) {
        row_count++;
        return true;
    });
    TEST_ASSERT(row_count == 1, "Should still have only 1 row after rollback");
    
    // Test 3: Automatic rollback on destruction
    {
        SQLiteTransaction transaction(db);
        result = db.execute("INSERT INTO transaction_test (value) VALUES (300)");
        TEST_ASSERT(result == true, "Insert in transaction");
        // Transaction destructor will automatically rollback
    }
    
    // Verify automatic rollback
    row_count = 0;
    result = db.execute("SELECT * FROM transaction_test", [&row_count](const SQLiteDatabase::Row& row) {
        row_count++;
        return true;
    });
    TEST_ASSERT(row_count == 1, "Should still have only 1 row after automatic rollback");
    
    db.close();
    cleanupTestDatabase("test_transactions.db");
}

void test_error_handling() {
    std::cout << "\n=== Testing Error Handling ===" << std::endl;
    
    // Setup: Create and open test database
    cleanupTestDatabase("test_errors.db");
    bool created = SQLiteDatabase::createdb("test_errors.db");
    TEST_ASSERT(created == true, "Setup: Create test database");
    
    SQLiteDatabase db;
    bool opened = db.open("test_errors.db");
    TEST_ASSERT(opened == true, "Setup: Open test database");
    
    // Test 1: Invalid SQL should fail
    bool result = db.execute("INVALID SQL STATEMENT");
    TEST_ASSERT(result == false, "Invalid SQL should fail");
    
    // Test 2: Check error information
    int error_code = db.getLastErrorCode();
    std::string error_msg = db.getLastErrorMessage();
    TEST_ASSERT(error_code != 0, "Error code should be non-zero for invalid SQL");
    TEST_ASSERT(!error_msg.empty(), "Error message should not be empty");
    std::cout << "  Error code: " << error_code << ", Message: " << error_msg << std::endl;
    
    // Test 3: Operations on closed database
    db.close();
    result = db.execute("SELECT 1");
    TEST_ASSERT(result == false, "Operations on closed database should fail");
    
    // Test 4: Invalid prepared statement
    auto stmt = db.prepare("SELECT * FROM nonexistent_table");
    TEST_ASSERT(stmt == nullptr, "Prepare invalid statement should return nullptr");
    
    // Test 5: Open non-existent database without CREATE flag
    SQLiteDatabase db2;
    result = db2.open("nonexistent.db", static_cast<int>(SQLiteDatabase::OpenFlags::ReadOnly));
    TEST_ASSERT(result == false, "Open non-existent database should fail");
    
    cleanupTestDatabase("test_errors.db");
}

void test_data_types() {
    std::cout << "\n=== Testing Data Types ===" << std::endl;
    
    // Setup: Create and open test database
    cleanupTestDatabase("test_datatypes.db");
    bool created = SQLiteDatabase::createdb("test_datatypes.db");
    TEST_ASSERT(created == true, "Setup: Create test database");
    
    SQLiteDatabase db;
    bool opened = db.open("test_datatypes.db");
    TEST_ASSERT(opened == true, "Setup: Open test database");
    
    // Create test table with various data types
    bool result = db.execute("CREATE TABLE datatype_test (id INTEGER PRIMARY KEY, "
                            "int_val INTEGER, real_val REAL, text_val TEXT, blob_val BLOB, null_val)");
    TEST_ASSERT(result == true, "Setup: Create test table with various types");
    
    // Test 1: Insert various data types
    auto stmt = db.prepare("INSERT INTO datatype_test (int_val, real_val, text_val, blob_val, null_val) "
                           "VALUES (?, ?, ?, ?, ?)");
    TEST_ASSERT(stmt != nullptr, "Prepare INSERT statement");
    
    std::vector<uint8_t> test_blob{0xDE, 0xAD, 0xBE, 0xEF};
    stmt->bindInt64(1, 9223372036854775807LL); // Max int64
    stmt->bindDouble(2, 3.141592653589793);
    stmt->bindText(3, "Hello, SQLite! 🚀");
    stmt->bindBlob(4, test_blob);
    stmt->bindNull(5);
    
    bool stepped = stmt->step();
    TEST_ASSERT(stepped == false, "INSERT should succeed");
    
    // Test 2: Query and verify data types
    result = db.execute("SELECT * FROM datatype_test", [&test_blob](const SQLiteDatabase::Row& row) {
        // Test integer
        long long int_val = row.getInt64(1);
        TEST_ASSERT(int_val == 9223372036854775807LL, "Int64 value should match");
        TEST_ASSERT(row.getColumnType(1) == SQLiteDatabase::ColumnType::Integer, "Column 1 should be Integer type");
        
        // Test real
        double real_val = row.getDouble(2);
        TEST_ASSERT(std::abs(real_val - 3.141592653589793) < 1e-15, "Double value should match");
        TEST_ASSERT(row.getColumnType(2) == SQLiteDatabase::ColumnType::Float, "Column 2 should be Float type");
        
        // Test text
        std::string text_val = row.getText(3);
        TEST_ASSERT(text_val == "Hello, SQLite! 🚀", "Text value should match (including Unicode)");
        TEST_ASSERT(row.getColumnType(3) == SQLiteDatabase::ColumnType::Text, "Column 3 should be Text type");
        
        // Test blob
        auto blob_val = row.getBlob(4);
        TEST_ASSERT(blob_val == test_blob, "Blob value should match");
        TEST_ASSERT(row.getColumnType(4) == SQLiteDatabase::ColumnType::Blob, "Column 4 should be Blob type");
        
        // Test null
        TEST_ASSERT(row.isNull(5) == true, "Column 5 should be null");
        TEST_ASSERT(row.getColumnType(5) == SQLiteDatabase::ColumnType::Null, "Column 5 should be Null type");
        
        return true;
    });
    TEST_ASSERT(result == true, "Query with data type verification should succeed");
    
    db.close();
    cleanupTestDatabase("test_datatypes.db");
}

void test_column_access() {
    std::cout << "\n=== Testing Column Access Methods ===" << std::endl;
    
    // Setup: Create and open test database
    cleanupTestDatabase("test_columns.db");
    bool created = SQLiteDatabase::createdb("test_columns.db");
    TEST_ASSERT(created == true, "Setup: Create test database");
    
    SQLiteDatabase db;
    bool opened = db.open("test_columns.db");
    TEST_ASSERT(opened == true, "Setup: Open test database");
    
    // Create test table
    bool result = db.execute("CREATE TABLE column_test (id INTEGER, name TEXT, value REAL)");
    TEST_ASSERT(result == true, "Setup: Create test table");
    
    // Insert test data
    result = db.execute("INSERT INTO column_test VALUES (42, 'test_column', 2.718)");
    TEST_ASSERT(result == true, "Setup: Insert test data");
    
    // Test column access by name and index
    result = db.execute("SELECT * FROM column_test", [](const SQLiteDatabase::Row& row) {
        // Test column count
        int col_count = row.getColumnCount();
        TEST_ASSERT(col_count == 3, "Should have 3 columns");
        
        // Test column names
        TEST_ASSERT(row.getColumnName(0) == "id", "Column 0 name should be 'id'");
        TEST_ASSERT(row.getColumnName(1) == "name", "Column 1 name should be 'name'");
        TEST_ASSERT(row.getColumnName(2) == "value", "Column 2 name should be 'value'");
        
        // Test column index lookup
        TEST_ASSERT(row.getColumnIndex("id") == 0, "Column 'id' should be at index 0");
        TEST_ASSERT(row.getColumnIndex("name") == 1, "Column 'name' should be at index 1");
        TEST_ASSERT(row.getColumnIndex("value") == 2, "Column 'value' should be at index 2");
        
        // Test access by column name
        TEST_ASSERT(row.getInt("id") == 42, "Access by name: id should be 42");
        TEST_ASSERT(row.getText("name") == "test_column", "Access by name: name should match");
        TEST_ASSERT(std::abs(row.getDouble("value") - 2.718) < 0.001, "Access by name: value should match");
        
        return true;
    });
    TEST_ASSERT(result == true, "Column access test should succeed");
    
    db.close();
    cleanupTestDatabase("test_columns.db");
}

int main() {
    std::cout << "🧪 LeafraSDK SQLite Unit Tests - Advanced Operations" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    // Initialize logging
    LEAFRA_INFO() << "Starting SQLite advanced unit tests";
    
    // Run all tests
    test_prepared_statements();
    test_transactions();
    test_error_handling();
    test_data_types();
    test_column_access();
    
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