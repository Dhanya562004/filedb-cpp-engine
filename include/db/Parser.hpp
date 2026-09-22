#ifndef FILEDB_PARSER_HPP
#define FILEDB_PARSER_HPP

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "Models.hpp"

namespace filedb {

class Parser {
public:
    static bool parse(const std::string &sql, AST &out_ast, std::string &error_msg);

    static std::string trim(const std::string &str);
    static std::string to_lower(const std::string &str);
    static std::string to_upper(const std::string &str);
    static bool starts_with_prefix(const std::string &str, const std::string &prefix);

private:
    static bool parse_create(const std::string &sql, AST_Create &ast, std::string &error_msg);
    static bool parse_insert(const std::string &sql, AST_Insert &ast, std::string &error_msg);
    static bool parse_select(const std::string &sql, AST_Select &ast, std::string &error_msg);
    static bool parse_update(const std::string &sql, AST_Update &ast, std::string &error_msg);
    static bool parse_delete(const std::string &sql, AST_Delete &ast, std::string &error_msg);
    static bool parse_condition(const std::string &cond_str, Condition &cond);

    static std::pair<int, int> find_top_level_parens(const std::string &str);
    static std::vector<std::string> split_top_level(const std::string &str, char delim);
};

} // namespace filedb

#endif // FILEDB_PARSER_HPP
