#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <functional>
#include "../include/db/ExecutionEngine.hpp"
#include "../include/db/Logger.hpp"

using namespace filedb;

void run_test(const std::string &test_name, std::function<bool()> test_func) {
    std::cout << "[TEST] Running " << test_name << "... ";
    try {
        if (test_func()) {
            std::cout << "PASSED\n";
        } else {
            std::cout << "FAILED\n";
        }
    } catch (const std::exception &e) {
        std::cout << "FAILED (Exception: " << e.what() << ")\n";
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "         FileDB++ Automated Unit Test Suite            \n";
    std::cout << "=======================================================\n\n";

    ExecutionEngine engine;

    // Cleanup existing test tables if any
    engine.execute_query("DELETE FROM test_students;");

    // Test 1: CREATE TABLE
    run_test("1. CREATE TABLE with Primary Key", [&]() {
        QueryResult res = engine.execute_query("CREATE TABLE test_students (id INT PRIMARY KEY, name TEXT, marks INT);");
        return res.success || res.error_message.find("already exists") != std::string::npos;
    });

    // Test 2: INSERT VALID ROWS
    run_test("2. INSERT Valid Records", [&]() {
        QueryResult r1 = engine.execute_query("INSERT INTO test_students VALUES (1, 'Alice', 95);");
        QueryResult r2 = engine.execute_query("INSERT INTO test_students VALUES (2, 'Bob', 85);");
        QueryResult r3 = engine.execute_query("INSERT INTO test_students VALUES (3, 'Charlie', 75);");
        return r1.success && r2.success && r3.success;
    });

    // Test 3: DUPLICATE PRIMARY KEY REJECTION
    run_test("3. REJECT Duplicate Primary Key", [&]() {
        QueryResult r = engine.execute_query("INSERT INTO test_students VALUES (1, 'DuplicateAlice', 90);");
        return !r.success && r.error_message.find("Duplicate Primary Key") != std::string::npos;
    });

    // Test 4: SELECT QUERY & INDEX LOOKUP
    run_test("4. SELECT with Primary Key Lookup", [&]() {
        QueryResult r = engine.execute_query("SELECT * FROM test_students WHERE id = 1;");
        return r.success && r.rows.size() == 1 && r.rows[0][1] == "Alice";
    });

    // Test 5: UPDATE RECORD
    run_test("5. UPDATE Record Marks", [&]() {
        QueryResult r1 = engine.execute_query("UPDATE test_students SET marks = 99 WHERE id = 1;");
        QueryResult r2 = engine.execute_query("SELECT marks FROM test_students WHERE id = 1;");
        return r1.success && r2.success && r2.rows.size() == 1 && r2.rows[0][0] == "99";
    });

    // Test 6: DELETE RECORD
    run_test("6. DELETE Record", [&]() {
        QueryResult r1 = engine.execute_query("DELETE FROM test_students WHERE id = 2;");
        QueryResult r2 = engine.execute_query("SELECT * FROM test_students WHERE id = 2;");
        return r1.success && r2.success && r2.rows.empty();
    });

    // Test 7: AI NLP TRANSLATION
    run_test("7. AI NLP to SQL Conversion", [&]() {
        QueryResult r = engine.execute_query("NLP show test_students with marks > 80");
        return r.success && !r.rows.empty();
    });

    // Test 8: INDEX VS SCAN TIMING COMPARISON BENCHMARK
    run_test("8. Performance Comparison (Index ON vs Index OFF)", [&]() {
        // Insert 100 dummy rows
        for (int i = 10; i < 110; ++i) {
            engine.execute_query("INSERT INTO test_students VALUES (" + std::to_string(i) + ", 'Student" + std::to_string(i) + "', 80);");
        }

        engine.set_indexing_enabled(true);
        QueryResult r_indexed = engine.execute_query("SELECT * FROM test_students WHERE id = 50;");

        engine.set_indexing_enabled(false);
        QueryResult r_scan = engine.execute_query("SELECT * FROM test_students WHERE id = 50;");
        engine.set_indexing_enabled(true);

        std::cout << "\n    --> Indexed Time : " << r_indexed.execution_time_ms << " ms (Strategy: " << r_indexed.plan.strategy_to_string() << ")"
                  << "\n    --> Full Scan Time: " << r_scan.execution_time_ms << " ms (Strategy: " << r_scan.plan.strategy_to_string() << ")\n";

        return r_indexed.success && r_scan.success && (r_indexed.plan.strategy != r_scan.plan.strategy);
    });

    std::cout << "\n=======================================================\n";
    std::cout << "             All Unit Tests Completed!                 \n";
    std::cout << "=======================================================\n";
    return 0;
}
