#ifndef HELPER
#define HELPER

#include <fstream>
#include <filesystem>
#include <algorithm>
#include "models.cpp"

using namespace std;
namespace fs = filesystem;

class Helper
{
public:
    static string trim(const string &str)
    {
        int start(0);
        while (start < str.size() && isspace(str[start]))
            ++start;

        if (start == str.size())
            return "";

        int end(str.size() - 1);
        while (end > start && isspace(str[end]))
            --end;

        return str.substr(start, end - start + 1);
    }
    static string to_lower(const string &str)
    {
        string result(str);
        for (char &c : result)
            c = tolower(c);
        return result;
    }

    static string to_upper(const string &str)
    {
        string result(str);
        for (char &c : result)
            c = toupper(c);
        return result;
    }

    static bool starts_with_prefix(const string &str, const string &prefix)
    {
        string lower_str(to_lower(str)),
            lower_prefix(to_lower(prefix));

        if (lower_str.size() < lower_prefix.size())
            return false;

        return lower_str.compare(0, lower_prefix.size(), lower_prefix) == 0;
    }
    static pair<int, int> find_top_level_parens(const string &str)
    {
        bool in_single_quote(false),
            in_double_quote(false);
        int depth(0), start(-1), end(-1);

        for (int i(0); i < str.size(); ++i)
        {
            char c(str[i]);

            if (c == '\'' && !in_double_quote)
            {
                in_single_quote = !in_single_quote;
                continue;
            }
            if (c == '"' && !in_single_quote)
            {
                in_double_quote = !in_double_quote;
                continue;
            }

            if (in_single_quote || in_double_quote)
                continue;

            if (c == '(')
            {
                if (!depth)
                    start = i;
                ++depth;
            }
            else if (c == ')')
            {
                --depth;
                if (!depth)
                {
                    end = i;
                    break;
                }
                if (depth < 0)
                    return {-1, -1};
            }
        }

        if (start > -1 && end > -1)
            return {start, end};

        return {-1, -1};
    }
    static vector<string> split_commas_respecting_quotes(const string &str)
    {
        vector<string> result;
        string current;
        bool in_single_quote(false), in_double_quote(false);
        int paren_depth(0);

        for (int i(0); i < str.size(); ++i)
        {
            char c(str[i]);

            if (c == '\'' && !in_double_quote)
            {
                in_single_quote = !in_single_quote;
                current += c;
                continue;
            }
            if (c == '"' && !in_single_quote)
            {
                in_double_quote = !in_double_quote;
                current += c;
                continue;
            }

            if (!in_single_quote && !in_double_quote)
            {
                if (c == '(')
                {
                    ++paren_depth;
                    current += c;
                    continue;
                }
                if (c == ')')
                {
                    if (paren_depth > 0)
                        --paren_depth;
                    current += c;
                    continue;
                }
                if (c == ',' && !paren_depth)
                {
                    result.push_back(trim(current));
                    current.clear();
                    continue;
                }
            }

            current += c;
        }
        current = trim(current);
        if (!current.empty())
            result.push_back(current);

        return result;
    }
    static vector<string> split_spaces_respecting_quotes(const string &str)
    {
        vector<string> tokens;
        string current;
        bool in_single_quote(false), in_double_quote(false);

        for (int i(0); i < str.size(); ++i)
        {
            char c(str[i]);

            if (c == '\'' && !in_double_quote)
            {
                in_single_quote = !in_single_quote;
                current += c;
                continue;
            }
            if (c == '"' && !in_single_quote)
            {
                in_double_quote = !in_double_quote;
                current += c;
                continue;
            }

            if (!in_single_quote && !in_double_quote && isspace(c))
            {
                if (!current.empty())
                    tokens.push_back(current), current.clear();
            }
            else
                current += c;
        }

        if (!current.empty())
            tokens.push_back(current);

        return tokens;
    }
    static vector<string> parse_column_name_list(const string &str)
    {
        vector<string> columns;
        vector<string> parts = split_commas_respecting_quotes(str);

        for (auto &part : parts)
            columns.push_back(trim(part));

        return columns;
    }
    static fs::path csv_path(const string &table_name)
    {
        return fs::path("../data") / table_name / (table_name + ".csv");
    }
    static fs::path meta_path(const string &table_name)
    {
        return fs::path("../data") / table_name / (table_name + ".meta");
    }
    static void ensure_data_dir()
    {
        fs::path data_dir("../data");
        if (!fs::exists(data_dir))
            fs::create_directory(data_dir);
    }
    static bool create_csv_header(const string &table_name, const vector<string> &columns)
    {
        ensure_data_dir();
        fs::path table_dir(fs::path("../data") / table_name);
        if (!fs::exists(table_dir))
            fs::create_directories(table_dir);

        fs::path path(csv_path(table_name));

        if (fs::exists(path))
            return false;

        ofstream file(path);
        if (!file.is_open())
            return false;

        for (int i(0); i < columns.size(); ++i)
        {
            string col_name(columns[i]);
            replace(col_name.begin(), col_name.end(), ',', '_'); // addr,city => addr_city
            file << col_name;

            if (i + 1 < columns.size())
                file << ",";
        }
        file << "\n";
        file.close();

        return true;
    }
    static bool write_meta(const string &table_name, const vector<Column> &columns,
                           const vector<string> &primary_key_cols)
    {
        ensure_data_dir();
        fs::path table_dir(fs::path("../data") / table_name);
        if (!fs::exists(table_dir))
            fs::create_directories(table_dir);

        fs::path path(meta_path(table_name));

        if (fs::exists(path))
            return false;

        ofstream file(path);
        if (!file.is_open())
            return false;

        file << "columns:\n";
        for (const auto &col : columns)
        {
            file << col.get_name() << "|"
                 << col.get_type() << "|"
                 << col.get_char_length() << "|"
                 << (col.is_null() ? "1" : "0") << "\n";
        }

        file << "pk:";
        for (int i(0); i < primary_key_cols.size(); ++i)
        {
            file << primary_key_cols[i];
            if (i + 1 < primary_key_cols.size())
                file << ",";
        }
        file << "\n";
        file.close();

        return true;
    }
    static void show_help()
    {
        cout << "\n================================================================\n";
        cout << "         Mini Database Engine - Command Reference\n";
        cout << "================================================================\n\n";

        cout << "--- SQL COMMANDS -----------------------------------------------\n\n";

        cout << ">> CREATE TABLE - Create a new table schema\n"
             << "  Syntax:\n"
             << "    CREATE TABLE table_name (\n"
             << "      col_name TYPE [NOT NULL] [PRIMARY KEY],\n"
             << "      ...,\n"
             << "      [PRIMARY KEY (col1, col2, ...)]\n"
             << "    );\n\n"
             << "  Data Types:\n"
             << "    INT, DOUBLE, DATE - numeric and date types\n"
             << "    VARCHAR(n), CHAR(n), TEXT - text types (all stored as VARCHAR)\n\n"
             << "  Examples:\n"
             << "    CREATE TABLE students (id INT PRIMARY KEY, name TEXT NOT NULL, gpa DOUBLE);\n"
             << "    CREATE TABLE orders (user_id INT, order_id INT, PRIMARY KEY(user_id, order_id));\n"
             << "    CREATE TABLE users (username CHAR(20) PRIMARY KEY, email VARCHAR(100));\n\n";

        cout << ">> INSERT - Add new rows to a table\n"
             << "  Syntax:\n"
             << "    INSERT INTO table_name VALUES (value1, value2, ...);\n\n"
             << "  Features:\n"
             << "    * Omit trailing values for nullable columns (auto-filled with NULL)\n"
             << "    * NOT NULL columns must have values or insertion fails\n"
             << "    * Primary key uniqueness is enforced automatically\n\n"
             << "  Examples:\n"
             << "    INSERT INTO students VALUES (1, 'Alice', 3.8);\n"
             << "    INSERT INTO students VALUES (2, 'Bob');           -- gpa becomes NULL\n"
             << "    INSERT INTO orders VALUES (101, 5001, '2025-12-25', 'Laptop');\n\n";

        cout << ">> SELECT - Query and retrieve data\n"
             << "  Syntax:\n"
             << "    SELECT * | col1, col2, ... FROM table_name \n"
             << "      [WHERE condition]\n"
             << "      [GROUP BY col1, col2, ...]\n"
             << "      [HAVING aggregate_condition];\n\n"
             << "  Features:\n"
             << "    * Use * to select all columns\n"
             << "    * Specify column names for partial selection\n"
             << "    * WHERE clause filters rows before grouping\n"
             << "    * GROUP BY groups rows by column values\n"
             << "    * HAVING filters groups after aggregation\n"
             << "    * Results displayed in formatted table view\n\n"
             << "  Examples:\n"
             << "    SELECT * FROM students;\n"
             << "    SELECT name, gpa FROM students WHERE gpa > 3.5;\n"
             << "    SELECT * FROM orders WHERE user_id = 101;\n"
             << "    SELECT username FROM users WHERE email = 'alice@example.com';\n\n";

        cout << ">> UPDATE - Modify existing rows\n"
             << "  Syntax:\n"
             << "    UPDATE table_name SET col1=val1, col2=val2, ... WHERE condition;\n\n"
             << "  Features:\n"
             << "    * Updates only rows matching WHERE condition\n"
             << "    * Can update multiple columns in one command\n"
             << "    * Primary key updates are allowed but must remain unique\n"
             << "    * Changes are persisted to CSV files automatically\n\n"
             << "  Examples:\n"
             << "    UPDATE students SET gpa = 3.9 WHERE id = 1;\n"
             << "    UPDATE students SET name = 'Robert', gpa = 4.0 WHERE id = 2;\n"
             << "    UPDATE orders SET status = 'shipped' WHERE order_id > 5000;\n\n";

        cout << ">> DELETE - Remove rows from a table\n"
             << "  Syntax:\n"
             << "    DELETE FROM table_name WHERE condition;\n\n"
             << "  Features:\n"
             << "    * Removes only rows matching WHERE condition\n"
             << "    * Deletions are persisted to CSV files automatically\n"
             << "    * Use with caution - no undo functionality\n\n"
             << "  Examples:\n"
             << "    DELETE FROM students WHERE id = 3;\n"
             << "    DELETE FROM orders WHERE status = 'cancelled';\n"
             << "    DELETE FROM users WHERE gpa < 2.0;\n\n";

        cout << ">> GROUP BY & HAVING - Aggregate and group data\n"
             << "  Syntax:\n"
             << "    SELECT col1, AGG_FUNC(col2), ... FROM table_name\n"
             << "      [WHERE condition]\n"
             << "      GROUP BY col1, col2, ...\n"
             << "      [HAVING aggregate_condition];\n\n"
             << "  Aggregate Functions:\n"
             << "    COUNT(*)      Count all rows in group\n"
             << "    COUNT(col)    Count non-NULL values in column\n"
             << "    SUM(col)      Sum numeric values\n"
             << "    AVG(col)      Average of numeric values\n"
             << "    MIN(col)      Minimum value\n"
             << "    MAX(col)      Maximum value\n\n"
             << "  Features:\n"
             << "    * GROUP BY groups rows by specified columns\n"
             << "    * Aggregate functions operate on each group\n"
             << "    * HAVING filters groups based on aggregate results\n"
             << "    * WHERE filters before grouping, HAVING filters after\n\n"
             << "  Examples:\n"
             << "    SELECT department, COUNT(*) FROM employees GROUP BY department;\n"
             << "    SELECT department, AVG(salary) FROM employees GROUP BY department;\n"
             << "    SELECT department, AVG(salary) FROM employees \n"
             << "      GROUP BY department HAVING AVG(salary) > 55000;\n"
             << "    SELECT category, COUNT(*), SUM(price) FROM products GROUP BY category;\n"
             << "    SELECT major, AVG(gpa) FROM students \n"
             << "      GROUP BY major HAVING AVG(gpa) >= 3.5;\n"
             << "    SELECT customer_name, SUM(total_price) FROM orders \n"
             << "      WHERE order_date > '2025-01-01'\n"
             << "      GROUP BY customer_name HAVING SUM(total_price) > 500;\n\n";

        cout << "----------------------------------------------------------------\n\n";

        cout << "--- DATA TYPES -------------------------------------------------\n\n"
             << "  INT          Integer numbers (e.g., 42, -10, 0)\n"
             << "  DOUBLE       Decimal numbers (e.g., 3.14, 99.99, -0.5)\n"
             << "  VARCHAR(n)   Variable-length strings (e.g., VARCHAR(50))\n"
             << "  CHAR(n)      Text (stored as VARCHAR)\n"
             << "  TEXT         Text (stored as VARCHAR)\n"
             << "  DATE         Date values in YYYY-MM-DD format (e.g., '2025-12-31')\n"
             << "  Note: CHAR, VARCHAR, and TEXT are all normalized to VARCHAR in storage\n\n";

        cout << "--- WHERE CLAUSE OPERATORS -------------------------------------\n\n"
             << "  =            Equal to                  (e.g., id = 5)\n"
             << "  !=           Not equal to              (e.g., status != 'pending')\n"
             << "  >            Greater than              (e.g., price > 100)\n"
             << "  <            Less than                 (e.g., age < 30)\n"
             << "  >=           Greater than or equal     (e.g., gpa >= 3.0)\n"
             << "  <=           Less than or equal        (e.g., quantity <= 50)\n\n";

        cout << "--- SPECIAL COMMANDS -------------------------------------------\n\n"
             << "  help, ?      Display this help message\n"
             << "  exit, quit   Exit the database engine\n\n";

        cout << "--- IMPORTANT NOTES --------------------------------------------\n\n"
             << "  * Strings must be enclosed in single quotes: 'text'\n"
             << "  * Dates must be in YYYY-MM-DD format: '2025-12-31'\n"
             << "  * Commands are case-insensitive: CREATE = create\n"
             << "  * Semicolons are optional at end of statements\n"
             << "  * Columns are nullable by default (use NOT NULL to require values)\n"
             << "  * Omitted INSERT values default to NULL (for nullable columns only)\n"
             << "  * Data is automatically persisted to ../data/table_name/ directory\n"
             << "  * Tables are auto-loaded from disk on engine startup\n"
             << "  * Primary keys enforce uniqueness (single or composite keys supported)\n\n";

        cout << "================================================================\n";
        cout << "Version: 1.0 | Features: CREATE, INSERT, SELECT, UPDATE, DELETE, NOT NULL\n";
    }

    static void load_existing_tables(Catalog *catalog)
    {
        fs::path data_dir("../data");
        if (!fs::exists(data_dir) || !fs::is_directory(data_dir))
            return;

        for (const auto &entry : fs::directory_iterator(data_dir))
        {
            if (!entry.is_directory())
                continue;

            string table_name(entry.path().filename().string());
            fs::path meta_file(entry.path() / (table_name + ".meta"));

            if (!fs::exists(meta_file))
                continue;

            ifstream file(meta_file);
            if (!file.is_open())
                continue;

            vector<Column> columns;
            vector<string> pk_cols;
            string line;

            while (getline(file, line))
            {
                if (line == "columns:")
                    continue;

                if (line.find("pk:") == 0)
                {
                    string pk_list(line.substr(3));
                    pk_cols = Helper::split_commas_respecting_quotes(pk_list);
                    for (auto &pk : pk_cols)
                        pk = Helper::trim(pk);
                    break;
                }

                int pos1(line.find('|')),
                    pos2(line.find('|', pos1 + 1)),
                    pos3(line.find('|', pos2 + 1));
                if (pos1 != string::npos && pos2 != string::npos)
                {
                    string name(Helper::trim(line.substr(0, pos1)));
                    string type(Helper::trim(line.substr(pos1 + 1, pos2 - pos1 - 1)));
                    string len_str(Helper::trim(line.substr(pos2 + 1, pos3 != string::npos ? pos3 - pos2 - 1 : string::npos)));
                    int len(stoi(len_str));
                    if (!len)
                        len = 1;

                    bool is_nullable(true);
                    if (pos3 != string::npos)
                    {
                        string null_str(Helper::trim(line.substr(pos3 + 1)));
                        is_nullable = (null_str == "1");
                    }

                    columns.emplace_back(name, type, false, len, is_nullable);
                }
            }
            file.close();

            if (!columns.empty())
            {
                Table *t(new Table(table_name, columns, pk_cols));

                fs::path csv_file(entry.path() / (table_name + ".csv"));
                if (fs::exists(csv_file))
                {
                    ifstream csv(csv_file);
                    string header;
                    getline(csv, header);

                    string data_line;
                    while (getline(csv, data_line))
                    {
                        if (data_line.empty())
                            continue;

                        auto values(Helper::split_commas_respecting_quotes(data_line));
                        if (values.size() != columns.size())
                            continue;

                        Row row;
                        for (int i(0); i < values.size(); ++i)
                        {
                            string val(Helper::trim(values[i]));
                            string type(columns[i].get_type());

                            if (val == "NULL")
                                row.push_back(Value());
                            else if (type == "INT")
                                row.push_back(Value(stoi(val)));
                            else if (type == "DOUBLE")
                                row.push_back(Value(stod(val)));
                            else if (type == "DATE")
                            {
                                int y(0), m(0), d(0);
                                sscanf(val.c_str(), "%d-%d-%d", &y, &m, &d);
                                row.push_back(Value(Date(y, m, d)));
                            }
                            else
                                row.push_back(Value(val));
                        }

                        try
                        {
                            t->insert_row(row);
                        }
                        catch (const exception &e)
                        {
                            continue;
                        }
                    }
                    csv.close();
                }

                catalog->addTable(t);
            }
        }
    }
};

#endif