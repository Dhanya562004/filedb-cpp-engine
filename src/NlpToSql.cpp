#include "../include/db/NlpToSql.hpp"
#include <iostream>
#include <sstream>

namespace filedb {

std::string NlpToSql::trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::string NlpToSql::to_lower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool NlpToSql::is_natural_language(const std::string &input_str) {
    std::string lower = to_lower(trim(input_str));
    if (lower.rfind("show", 0) == 0 ||
        lower.rfind("get", 0) == 0 ||
        lower.rfind("find", 0) == 0 ||
        lower.rfind("fetch", 0) == 0 ||
        lower.rfind("display", 0) == 0 ||
        lower.rfind("list", 0) == 0 ||
        lower.rfind("remove", 0) == 0) {
        return true;
    }
    return false;
}

std::string NlpToSql::convert_to_sql(const std::string &nl_query) {
    std::string clean = trim(nl_query);
    if (clean.empty()) return "";

    std::string lower = to_lower(clean);

    // Remove trailing dot or semicolon if present
    if (clean.back() == '.' || clean.back() == ';') {
        clean.pop_back();
        clean = trim(clean);
        lower = to_lower(clean);
    }

    // Pattern 1: "show/get/find/list/fetch/display [all] <table> with/where <col> <op> <val>"
    // Example: "show students with marks > 80" -> "SELECT * FROM students WHERE marks > 80;"
    std::regex where_regex(R"((show|get|find|list|fetch|display)\s+(all\s+)?([a-zA-Z0-9_]+)\s+(with|where|having)\s+([a-zA-Z0-9_]+)\s*(=|!=|>|<|>=|<=)\s*(.+))", std::regex::icase);
    std::smatch match;
    if (std::regex_match(clean, match, where_regex)) {
        std::string table = match[3].str();
        std::string col = match[5].str();
        std::string op = match[6].str();
        std::string val = match[7].str();
        return "SELECT * FROM " + table + " WHERE " + col + " " + op + " " + val + ";";
    }

    // Pattern 2: "show/get/find/list/fetch [all] <table>"
    // Example: "show all students" -> "SELECT * FROM students;"
    std::regex simple_select(R"((show|get|find|list|fetch|display)\s+(all\s+)?([a-zA-Z0-9_]+))", std::regex::icase);
    if (std::regex_match(clean, match, simple_select)) {
        std::string table = match[3].str();
        return "SELECT * FROM " + table + ";";
    }

    // Pattern 3: "delete/remove <table> with/where <col> <op> <val>"
    // Example: "delete student with id = 5" -> "DELETE FROM student WHERE id = 5;"
    std::regex delete_regex(R"((delete|remove)\s+(from\s+)?([a-zA-Z0-9_]+)\s+(with|where)\s+([a-zA-Z0-9_]+)\s*(=|!=|>|<|>=|<=)\s*(.+))", std::regex::icase);
    if (std::regex_match(clean, match, delete_regex)) {
        std::string table = match[3].str();
        std::string col = match[5].str();
        std::string op = match[6].str();
        std::string val = match[7].str();
        return "DELETE FROM " + table + " WHERE " + col + " " + op + " " + val + ";";
    }

    // Pattern 4: "insert into <table> <val1>, <val2>..."
    // Example: "insert into students 1, 'Alice', 95" -> "INSERT INTO students VALUES (1, 'Alice', 95);"
    std::regex insert_regex(R"(insert\s+into\s+([a-zA-Z0-9_]+)\s+(values\s*)?\(?(.+)\)?)", std::regex::icase);
    if (std::regex_match(clean, match, insert_regex)) {
        std::string table = match[1].str();
        std::string vals = match[3].str();
        if (vals.front() != '(') vals = "(" + vals + ")";
        return "INSERT INTO " + table + " VALUES " + vals + ";";
    }

    // Fallback: If it's already an SQL query, return as is (with trailing semicolon)
    if (lower.rfind("select", 0) == 0 ||
        lower.rfind("insert", 0) == 0 ||
        lower.rfind("update", 0) == 0 ||
        lower.rfind("delete", 0) == 0 ||
        lower.rfind("create", 0) == 0) {
        return clean + (clean.back() == ';' ? "" : ";");
    }

    return clean;
}

} // namespace filedb
