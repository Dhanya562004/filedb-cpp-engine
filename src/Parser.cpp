#include "../include/db/Parser.hpp"
#include "../include/db/NlpToSql.hpp"
#include <iostream>

namespace filedb {

std::string Parser::trim(const std::string &str) {
    size_t start = 0;
    while (start < str.size() && isspace(static_cast<unsigned char>(str[start]))) ++start;
    if (start == str.size()) return "";
    size_t end = str.size() - 1;
    while (end > start && isspace(static_cast<unsigned char>(str[end]))) --end;
    return str.substr(start, end - start + 1);
}

std::string Parser::to_lower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string Parser::to_upper(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool Parser::starts_with_prefix(const std::string &str, const std::string &prefix) {
    std::string lower_str = to_lower(str);
    std::string lower_prefix = to_lower(prefix);
    if (lower_str.size() < lower_prefix.size()) return false;
    return lower_str.compare(0, lower_prefix.size(), lower_prefix) == 0;
}

std::pair<int, int> Parser::find_top_level_parens(const std::string &str) {
    bool in_sq = false, in_dq = false;
    int depth = 0, start = -1, end = -1;
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (c == '\'' && !in_dq) { in_sq = !in_sq; continue; }
        if (c == '"' && !in_sq) { in_dq = !in_dq; continue; }
        if (in_sq || in_dq) continue;

        if (c == '(') {
            if (depth == 0) start = static_cast<int>(i);
            ++depth;
        } else if (c == ')') {
            --depth;
            if (depth == 0) {
                end = static_cast<int>(i);
                break;
            }
            if (depth < 0) return {-1, -1};
        }
    }
    if (start > -1 && end > -1) return {start, end};
    return {-1, -1};
}

std::vector<std::string> Parser::split_top_level(const std::string &str, char delim) {
    std::vector<std::string> result;
    std::string current;
    bool in_sq = false, in_dq = false;
    int depth = 0;

    for (char c : str) {
        if (c == '\'' && !in_dq) { in_sq = !in_sq; current += c; continue; }
        if (c == '"' && !in_sq) { in_dq = !in_dq; current += c; continue; }
        if (in_sq || in_dq) { current += c; continue; }

        if (c == '(') ++depth;
        else if (c == ')') --depth;

        if (c == delim && depth == 0) {
            result.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        result.push_back(trim(current));
    }
    return result;
}

bool Parser::parse_condition(const std::string &cond_str, Condition &cond) {
    std::string s = trim(cond_str);
    std::vector<std::string> ops = {"<=", ">=", "!=", "<>", "==", "=", "<", ">"};
    for (const auto &op : ops) {
        size_t pos = s.find(op);
        if (pos != std::string::npos) {
            cond.lhs = trim(s.substr(0, pos));
            cond.op = op;
            cond.rhs = trim(s.substr(pos + op.size()));
            return !cond.lhs.empty() && !cond.rhs.empty();
        }
    }
    return false;
}

bool Parser::parse_create(const std::string &sql, AST_Create &ast, std::string &error_msg) {
    auto parens = find_top_level_parens(sql);
    if (parens.first == -1 || parens.second == -1) {
        error_msg = "Syntax error in CREATE TABLE: missing parentheses around column definitions";
        return false;
    }

    std::string header = trim(sql.substr(0, parens.first));
    std::string lower_hdr = to_lower(header);
    if (lower_hdr.rfind("create table", 0) != 0) {
        error_msg = "Syntax error: expected 'CREATE TABLE'";
        return false;
    }

    ast.table_name = trim(header.substr(12));
    if (ast.table_name.empty()) {
        error_msg = "Syntax error: missing table name in CREATE TABLE";
        return false;
    }

    std::string body = sql.substr(parens.first + 1, parens.second - parens.first - 1);
    auto items = split_top_level(body, ',');

    for (const auto &item : items) {
        std::string lower_item = to_lower(item);
        if (lower_item.rfind("primary key", 0) == 0) {
            auto pk_parens = find_top_level_parens(item);
            if (pk_parens.first != -1 && pk_parens.second != -1) {
                std::string pk_cols_str = item.substr(pk_parens.first + 1, pk_parens.second - pk_parens.first - 1);
                auto cols = split_top_level(pk_cols_str, ',');
                for (const auto &c : cols) ast.pk_columns.push_back(trim(c));
            }
        } else {
            std::stringstream ss(item);
            std::string cname, ctype;
            ss >> cname >> ctype;
            if (cname.empty() || ctype.empty()) continue;

            bool is_pk = false;
            std::string rem;
            std::getline(ss, rem);
            std::string lower_rem = to_lower(rem);
            if (lower_rem.find("primary key") != std::string::npos) {
                is_pk = true;
                ast.pk_columns.push_back(cname);
            }

            Column col(cname, to_upper(ctype), is_pk);
            ast.columns.push_back(col);
        }
    }
    return !ast.columns.empty();
}

bool Parser::parse_insert(const std::string &sql, AST_Insert &ast, std::string &error_msg) {
    std::string lower = to_lower(sql);
    size_t val_pos = lower.find("values");
    if (val_pos == std::string::npos) {
        error_msg = "Syntax error in INSERT INTO: missing 'VALUES' keyword";
        return false;
    }

    std::string header = trim(sql.substr(0, val_pos));
    std::string lower_hdr = to_lower(header);
    if (lower_hdr.rfind("insert into", 0) != 0) {
        error_msg = "Syntax error: expected 'INSERT INTO'";
        return false;
    }

    ast.table_name = trim(header.substr(11));

    std::string val_part = trim(sql.substr(val_pos + 6));
    auto parens = find_top_level_parens(val_part);
    if (parens.first == -1 || parens.second == -1) {
        error_msg = "Syntax error in INSERT INTO: missing parentheses around VALUES";
        return false;
    }

    std::string body = val_part.substr(parens.first + 1, parens.second - parens.first - 1);
    ast.raw_values = split_top_level(body, ',');
    return !ast.table_name.empty() && !ast.raw_values.empty();
}

bool Parser::parse_select(const std::string &sql, AST_Select &ast, std::string &error_msg) {
    std::string lower = to_lower(sql);
    size_t from_pos = lower.find(" from ");
    if (from_pos == std::string::npos) {
        error_msg = "Syntax error in SELECT: missing 'FROM' clause";
        return false;
    }

    std::string select_part = trim(sql.substr(6, from_pos - 6));
    ast.select_list = split_top_level(select_part, ',');

    std::string remainder = trim(sql.substr(from_pos + 6));
    std::string lower_rem = to_lower(remainder);

    size_t where_pos = lower_rem.find(" where ");
    size_t group_pos = lower_rem.find(" group by ");
    size_t having_pos = lower_rem.find(" having ");

    if (where_pos != std::string::npos) {
        ast.table_name = trim(remainder.substr(0, where_pos));
        size_t end_where = (group_pos != std::string::npos) ? group_pos :
                           ((having_pos != std::string::npos) ? having_pos : remainder.size());
        std::string where_clause = trim(remainder.substr(where_pos + 7, end_where - (where_pos + 7)));
        
        auto cond_strs = split_top_level(where_clause, ' ');
        // Parse conditions joined by AND
        std::string current_cond;
        for (const auto &token : cond_strs) {
            if (to_lower(token) == "and") {
                Condition cond;
                if (parse_condition(current_cond, cond)) ast.where.push_back(cond);
                current_cond.clear();
            } else {
                if (!current_cond.empty()) current_cond += " ";
                current_cond += token;
            }
        }
        if (!current_cond.empty()) {
            Condition cond;
            if (parse_condition(current_cond, cond)) ast.where.push_back(cond);
        }
    } else {
        size_t end_tbl = (group_pos != std::string::npos) ? group_pos :
                         ((having_pos != std::string::npos) ? having_pos : remainder.size());
        ast.table_name = trim(remainder.substr(0, end_tbl));
    }

    return !ast.table_name.empty();
}

bool Parser::parse_update(const std::string &sql, AST_Update &ast, std::string &error_msg) {
    std::string lower = to_lower(sql);
    size_t set_pos = lower.find(" set ");
    if (set_pos == std::string::npos) {
        error_msg = "Syntax error in UPDATE: missing 'SET' clause";
        return false;
    }

    ast.table_name = trim(sql.substr(7, set_pos - 7));

    std::string remainder = trim(sql.substr(set_pos + 5));
    std::string lower_rem = to_lower(remainder);
    size_t where_pos = lower_rem.find(" where ");

    std::string set_clause = (where_pos != std::string::npos) ? remainder.substr(0, where_pos) : remainder;
    auto set_assignments = split_top_level(set_clause, ',');
    for (const auto &assign : set_assignments) {
        size_t eq = assign.find('=');
        if (eq != std::string::npos) {
            ast.sets.emplace_back(trim(assign.substr(0, eq)), trim(assign.substr(eq + 1)));
        }
    }

    if (where_pos != std::string::npos) {
        std::string where_clause = trim(remainder.substr(where_pos + 7));
        Condition cond;
        if (parse_condition(where_clause, cond)) ast.where.push_back(cond);
    }

    return !ast.table_name.empty() && !ast.sets.empty();
}

bool Parser::parse_delete(const std::string &sql, AST_Delete &ast, std::string &error_msg) {
    std::string lower = to_lower(sql);
    size_t from_pos = lower.find("from ");
    if (from_pos == std::string::npos) {
        error_msg = "Syntax error in DELETE: missing 'FROM' clause";
        return false;
    }

    std::string remainder = trim(sql.substr(from_pos + 5));
    std::string lower_rem = to_lower(remainder);
    size_t where_pos = lower_rem.find(" where ");

    if (where_pos != std::string::npos) {
        ast.table_name = trim(remainder.substr(0, where_pos));
        std::string where_clause = trim(remainder.substr(where_pos + 7));
        Condition cond;
        if (parse_condition(where_clause, cond)) ast.where.push_back(cond);
    } else {
        ast.table_name = remainder;
    }

    return !ast.table_name.empty();
}

bool Parser::parse(const std::string &sql_in, AST &out_ast, std::string &error_msg) {
    std::string sql = trim(sql_in);
    if (sql.empty()) {
        error_msg = "Empty query input";
        return false;
    }

    if (sql.back() == ';') sql.pop_back();
    sql = trim(sql);

    std::string lower = to_lower(sql);

    if (starts_with_prefix(sql, "EXPLAIN ")) {
        out_ast.kind = ASTKind::EXPLAIN;
        AST_Explain exp;
        exp.inner_sql = trim(sql.substr(8));
        out_ast.node = exp;
        return true;
    }

    if (starts_with_prefix(sql, "NLP ")) {
        out_ast.kind = ASTKind::NLP;
        AST_Nlp nlp;
        nlp.natural_language_query = trim(sql.substr(4));
        out_ast.node = nlp;
        return true;
    }

    if (NlpToSql::is_natural_language(sql)) {
        out_ast.kind = ASTKind::NLP;
        AST_Nlp nlp;
        nlp.natural_language_query = sql;
        out_ast.node = nlp;
        return true;
    }

    if (starts_with_prefix(sql, "CREATE")) {
        out_ast.kind = ASTKind::CREATE;
        AST_Create ast;
        if (parse_create(sql, ast, error_msg)) {
            out_ast.node = ast;
            return true;
        }
        return false;
    }

    if (starts_with_prefix(sql, "INSERT")) {
        out_ast.kind = ASTKind::INSERT;
        AST_Insert ast;
        if (parse_insert(sql, ast, error_msg)) {
            out_ast.node = ast;
            return true;
        }
        return false;
    }

    if (starts_with_prefix(sql, "SELECT")) {
        out_ast.kind = ASTKind::SELECT;
        AST_Select ast;
        if (parse_select(sql, ast, error_msg)) {
            out_ast.node = ast;
            return true;
        }
        return false;
    }

    if (starts_with_prefix(sql, "UPDATE")) {
        out_ast.kind = ASTKind::UPDATE;
        AST_Update ast;
        if (parse_update(sql, ast, error_msg)) {
            out_ast.node = ast;
            return true;
        }
        return false;
    }

    if (starts_with_prefix(sql, "DELETE")) {
        out_ast.kind = ASTKind::_DELETE;
        AST_Delete ast;
        if (parse_delete(sql, ast, error_msg)) {
            out_ast.node = ast;
            return true;
        }
        return false;
    }

    error_msg = "Unknown or unsupported SQL statement: " + sql;
    return false;
}

} // namespace filedb
