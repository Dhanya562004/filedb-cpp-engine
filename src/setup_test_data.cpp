#include "../include/CreateParse.cpp"
#include "../include/InsertParser.cpp"
#include <iostream>

using namespace std;

int main()
{
    cout << "\n================================================\n";
    cout << "     Setting Up Test Database Tables       \n";
    cout << "================================================\n\n";

    CreateParser create_parser;
    Catalog *catalog(&create_parser.catalog());
    InsertParser insert_parser(catalog);
    AST ast;

    // ============================================
    // Table 1: Users
    // ============================================
    cout << "Creating 'users' table...\n";
    bool success = create_parser.parse_and_create(
        "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50) NOT NULL, email TEXT, age INT);", ast);

    if (success)
    {
        cout << "✓ Users table created successfully\n";
        cout << "  Inserting sample data...\n";

        insert_parser.parse_and_insert("INSERT INTO users VALUES (1, 'Ahmed Ali', 'ahmed@example.com', 25);", ast);
        insert_parser.parse_and_insert("INSERT INTO users VALUES (2, 'Sara Mohamed', 'sara@example.com', 30);", ast);
        insert_parser.parse_and_insert("INSERT INTO users VALUES (3, 'John Smith', 'john@example.com', 28);", ast);
        insert_parser.parse_and_insert("INSERT INTO users VALUES (4, 'Emily Brown', 'emily@example.com', 35);", ast);
        insert_parser.parse_and_insert("INSERT INTO users VALUES (5, 'Mohamed Hassan', 'mohamed@example.com', 22);", ast);

        cout << "✓ 5 rows inserted into users\n\n";
    }

    // ============================================
    // Table 2: Products
    // ============================================
    cout << "Creating 'products' table...\n";
    success = create_parser.parse_and_create(
        "CREATE TABLE products (id INT PRIMARY KEY, name TEXT NOT NULL, price DOUBLE NOT NULL, stock INT, category TEXT);", ast);

    if (success)
    {
        cout << "✓ Products table created successfully\n";
        cout << "  Inserting sample data...\n";

        insert_parser.parse_and_insert("INSERT INTO products VALUES (101, 'Laptop', 999.99, 15, 'Electronics');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (102, 'Mouse', 25.50, 50, 'Electronics');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (103, 'Keyboard', 75.00, 30, 'Electronics');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (104, 'Monitor', 299.99, 20, 'Electronics');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (105, 'Desk Chair', 150.00, 10, 'Furniture');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (106, 'Desk Lamp', 35.99, 25, 'Furniture');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (107, 'Office Desk', 249.99, 8, 'Furniture');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (108, 'Headphones', 89.99, 40, 'Electronics');", ast);
        insert_parser.parse_and_insert("INSERT INTO products VALUES (109, 'Bookshelf', 120.00, 12, 'Furniture');", ast);

        cout << "✓ 9 rows inserted into products\n\n";
    }

    // ============================================
    // Table 3: Employees
    // ============================================
    cout << "Creating 'employees' table...\n";
    success = create_parser.parse_and_create(
        "CREATE TABLE employees (id INT PRIMARY KEY, name TEXT NOT NULL, salary DOUBLE NOT NULL, department TEXT NOT NULL, hire_date DATE);", ast);

    if (success)
    {
        cout << "✓ Employees table created successfully\n";
        cout << "  Inserting sample data...\n";

        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1001, 'Alice Johnson', 50000, 'IT', '2020-01-15');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1002, 'Bob Wilson', 60000, 'Sales', '2019-05-20');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1003, 'Charlie Davis', 55000, 'IT', '2021-03-10');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1004, 'Diana Martinez', 65000, 'HR', '2018-11-05');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1005, 'Eve Anderson', 70000, 'IT', '2017-08-22');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1006, 'Frank Thomas', 48000, 'Sales', '2022-02-14');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1007, 'Grace Lee', 52000, 'HR', '2020-09-30');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1008, 'Henry Clark', 75000, 'IT', '2019-07-12');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1009, 'Iris White', 56000, 'Sales', '2021-01-18');", ast);
        insert_parser.parse_and_insert("INSERT INTO employees VALUES (1010, 'Jack Brown', 62000, 'Sales', '2020-04-25');", ast);

        cout << "✓ 10 rows inserted into employees\n\n";
    }

    // ============================================
    // Table 4: Orders
    // ============================================
    cout << "Creating 'orders' table...\n";
    success = create_parser.parse_and_create(
        "CREATE TABLE orders (order_id INT PRIMARY KEY, customer_name TEXT NOT NULL, product_id INT NOT NULL, quantity INT NOT NULL, order_date DATE NOT NULL, total_price DOUBLE);", ast);

    if (success)
    {
        cout << "✓ Orders table created successfully\n";
        cout << "  Inserting sample data...\n";

        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5001, 'Ahmed Ali', 101, 1, '2025-12-01', 999.99);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5002, 'Sara Mohamed', 102, 2, '2025-12-02', 51.00);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5003, 'John Smith', 103, 1, '2025-12-03', 75.00);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5004, 'Emily Brown', 104, 1, '2025-12-04', 299.99);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5005, 'Mohamed Hassan', 105, 1, '2025-12-05', 150.00);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5006, 'Ahmed Ali', 102, 3, '2025-12-06', 76.50);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5007, 'Sara Mohamed', 103, 2, '2025-12-07', 150.00);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5008, 'Ahmed Ali', 108, 1, '2025-12-08', 89.99);", ast);
        insert_parser.parse_and_insert("INSERT INTO orders VALUES (5009, 'John Smith', 104, 2, '2025-12-09', 599.98);", ast);

        cout << "✓ 9 rows inserted into orders\n\n";
    }

    // ============================================
    // Table 5: Students
    // ============================================
    cout << "Creating 'students' table...\n";
    success = create_parser.parse_and_create(
        "CREATE TABLE students (student_id INT PRIMARY KEY, name TEXT NOT NULL, major TEXT, gpa DOUBLE, enrollment_date DATE);", ast);

    if (success)
    {
        cout << "✓ Students table created successfully\n";
        cout << "  Inserting sample data...\n";

        insert_parser.parse_and_insert("INSERT INTO students VALUES (2001, 'Omar Khaled', 'Computer Science', 3.8, '2022-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2002, 'Fatma Ibrahim', 'Engineering', 3.5, '2022-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2003, 'Youssef Ahmed', 'Business', 3.2, '2023-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2004, 'Nour Hassan', 'Computer Science', 3.9, '2021-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2005, 'Mina Fady', 'Medicine', 3.7, '2022-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2006, 'Layla Mostafa', 'Engineering', 3.6, '2023-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2007, 'Karim Said', 'Computer Science', 3.4, '2022-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2008, 'Hana Ahmed', 'Business', 3.8, '2021-09-01');", ast);
        insert_parser.parse_and_insert("INSERT INTO students VALUES (2009, 'Ziad Tarek', 'Engineering', 3.3, '2023-09-01');", ast);

        cout << "✓ 9 rows inserted into students\n\n";
    }

    // ============================================
    // Summary
    // ============================================
    cout << "================================================\n";
    cout << "        Test Database Setup Complete!      \n";
    cout << "================================================\n\n";

    cout << "Tables created:\n";
    cout << "  1. users      -  5 rows  (User information)\n";
    cout << "  2. products   -  9 rows  (Product catalog)\n";
    cout << "  3. employees  - 10 rows  (Employee records)\n";
    cout << "  4. orders     -  9 rows  (Customer orders)\n";
    cout << "  5. students   -  9 rows  (Student information)\n\n";

    cout << "Data files location: ../data/\n\n";

    cout << "Basic query examples:\n";
    cout << "  SELECT * FROM users;\n";
    cout << "  SELECT name, salary FROM employees WHERE department = 'IT';\n";
    cout << "  UPDATE products SET price = 899.99 WHERE id = 101;\n";
    cout << "  DELETE FROM orders WHERE order_id = 5001;\n";
    cout << "  INSERT INTO users VALUES (6, 'New User', 'new@example.com', 40);\n\n";

    cout << "GROUP BY and HAVING examples:\n";
    cout << "  SELECT department, COUNT(*) FROM employees GROUP BY department;\n";
    cout << "  SELECT department, AVG(salary) FROM employees GROUP BY department;\n";
    cout << "  SELECT department, AVG(salary) FROM employees GROUP BY department HAVING AVG(salary) > 55000;\n";
    cout << "  SELECT category, COUNT(*), SUM(price) FROM products GROUP BY category;\n";
    cout << "  SELECT customer_name, COUNT(*), SUM(total_price) FROM orders GROUP BY customer_name;\n";
    cout << "  SELECT customer_name, SUM(total_price) FROM orders GROUP BY customer_name HAVING SUM(total_price) > 500;\n";
    cout << "  SELECT major, AVG(gpa) FROM students GROUP BY major HAVING AVG(gpa) >= 3.5;\n\n";

    return 0;
}
