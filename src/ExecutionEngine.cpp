#include "../include/db/ExecutionEngine.hpp"
#include "../include/db/Parser.hpp"
#include "../include/db/NlpToSql.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace filedb {

std::string QueryResult::to_cli_table() const {
    if (!success) {
        return "ERROR: " + error_message;
    }
    if (columns.empty()) {
        std::stringstream ss;
        ss << (info_message.empty() ? "Query executed successfully." : info_message)
           << " (" << std::fixed << std::setprecision(3) << execution_time_ms << " ms)";
        return ss.str();
    }

    // Calculate column widths
    std::vector<size_t> col_widths(columns.size(), 0);
    for (size_t i = 0; i < columns.size(); ++i) {
        col_widths[i] = columns[i].size();
    }
    for (const auto &row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i < col_widths.size()) {
                col_widths[i] = std::max(col_widths[i], row[i].size());
            }
        }
    }

    std::stringstream ss;
    // Top border
    ss << "+";
    for (size_t w : col_widths) ss << std::string(w + 2, '-') << "+";
    ss << "\n";

    // Header row
    ss << "|";
    for (size_t i = 0; i < columns.size(); ++i) {
        ss << " " << std::left << std::setw(col_widths[i]) << columns[i] << " |";
    }
    ss << "\n";

    // Header separator
    ss << "+";
    for (size_t w : col_widths) ss << std::string(w + 2, '-') << "+";
    ss << "\n";

    // Data rows
    for (const auto &row : rows) {
        ss << "|";
        for (size_t i = 0; i < row.size(); ++i) {
            ss << " " << std::left << std::setw(col_widths[i]) << (i < row.size() ? row[i] : "") << " |";
        }
        ss << "\n";
    }

    // Bottom border
    ss << "+";
    for (size_t w : col_widths) ss << std::string(w + 2, '-') << "+";
    ss << "\n";

    ss << rows.size() << " row(s) in set (" << std::fixed << std::setprecision(3) << execution_time_ms << " ms)";
    return ss.str();
}

static std::string escape_json(const std::string &s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}

std::string QueryResult::to_json() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"success\": " << (success ? "true" : "false") << ",\n";
    ss << "  \"error_message\": \"" << escape_json(error_message) << "\",\n";
    ss << "  \"info_message\": \"" << escape_json(info_message) << "\",\n";
    ss << "  \"execution_time_ms\": " << execution_time_ms << ",\n";
    ss << "  \"rows_scanned\": " << rows_scanned << ",\n";
    ss << "  \"rows_affected\": " << rows_affected << ",\n";
    
    // Columns array
    ss << "  \"columns\": [";
    for (size_t i = 0; i < columns.size(); ++i) {
        ss << "\"" << escape_json(columns[i]) << "\"" << (i + 1 < columns.size() ? ", " : "");
    }
    ss << "],\n";

    // Rows array
    ss << "  \"rows\": [\n";
    for (size_t r = 0; r < rows.size(); ++r) {
        ss << "    [";
        for (size_t c = 0; c < rows[r].size(); ++c) {
            ss << "\"" << escape_json(rows[r][c]) << "\"" << (c + 1 < rows[r].size() ? ", " : "");
        }
        ss << "]" << (r + 1 < rows.size() ? ",\n" : "\n");
    }
    ss << "  ],\n";

    // Execution plan
    ss << "  \"plan\": {\n";
    ss << "    \"table_name\": \"" << escape_json(plan.table_name) << "\",\n";
    ss << "    \"strategy\": \"" << escape_json(plan.strategy_to_string()) << "\",\n";
    ss << "    \"index_used\": \"" << escape_json(plan.index_used) << "\",\n";
    ss << "    \"filter_condition\": \"" << escape_json(plan.filter_condition) << "\",\n";
    ss << "    \"total_table_rows\": " << plan.total_table_rows << ",\n";
    ss << "    \"rows_scanned\": " << plan.rows_scanned << ",\n";
    ss << "    \"rows_returned\": " << plan.rows_returned << ",\n";
    ss << "    \"indexing_enabled\": " << (plan.indexing_enabled ? "true" : "false") << "\n";
    ss << "  }\n";
    ss << "}";
    return ss.str();
}

ExecutionEngine::ExecutionEngine() {
    storage_mgr.load_all_tables(catalog);
    auto table_names = catalog.get_table_names();
    for (const auto &tname : table_names) {
        Table *t = catalog.getTable(tname);
        if (t) {
            index_mgr.build_indexes_for_table(*t);
        }
    }
}

Value ExecutionEngine::parse_raw_literal(const std::string &raw_in, const Column &col) {
    std::string raw = Parser::trim(raw_in);
    if (raw.size() >= 2 && ((raw.front() == '\'' && raw.back() == '\'') ||
                            (raw.front() == '"' && raw.back() == '"'))) {
        raw = raw.substr(1, raw.size() - 2);
    }
    if (raw == "NULL" || raw == "null") {
        return Value();
    }

    std::string type = col.get_type();
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);

    if (type == "INT" || type == "INTEGER") {
        try { return Value(std::stoi(raw)); } catch (...) { return Value(0); }
    } else if (type == "DOUBLE" || type == "FLOAT" || type == "DECIMAL") {
        try { return Value(std::stod(raw)); } catch (...) { return Value(0.0); }
    } else if (type == "CHAR" && raw.size() == 1) {
        return Value(raw[0]);
    } else if (type == "DATE") {
        int y = 0, m = 0, d = 0;
        if (sscanf(raw.c_str(), "%d-%d-%d", &y, &m, &d) == 3) {
            return Value(Date(y, m, d));
        }
        return Value(raw);
    }
    return Value(raw);
}

QueryResult ExecutionEngine::execute_query(const std::string &sql) {
    AST ast;
    std::string error_msg;
    if (!Parser::parse(sql, ast, error_msg)) {
        QueryResult res;
        res.success = false;
        res.error_message = error_msg;
        Logger::get_instance().log_error(sql, error_msg);
        return res;
    }
    return execute_ast(ast, sql);
}

QueryResult ExecutionEngine::execute_ast(const AST &ast, const std::string &original_sql) {
    switch (ast.kind) {
        case ASTKind::CREATE:
            return execute_create(std::get<AST_Create>(ast.node), original_sql);
        case ASTKind::INSERT:
            return execute_insert(std::get<AST_Insert>(ast.node), original_sql);
        case ASTKind::SELECT:
            return execute_select(std::get<AST_Select>(ast.node), original_sql);
        case ASTKind::UPDATE:
            return execute_update(std::get<AST_Update>(ast.node), original_sql);
        case ASTKind::_DELETE:
            return execute_delete(std::get<AST_Delete>(ast.node), original_sql);
        case ASTKind::EXPLAIN:
            return execute_explain(std::get<AST_Explain>(ast.node), original_sql);
        case ASTKind::NLP:
            return execute_nlp(std::get<AST_Nlp>(ast.node), original_sql);
        default: {
            QueryResult res;
            res.success = false;
            res.error_message = "Unknown AST node";
            Logger::get_instance().log_error(original_sql, res.error_message);
            return res;
        }
    }
}

QueryResult ExecutionEngine::execute_create(const AST_Create &ast, const std::string &sql) {
    auto start_time = std::chrono::high_resolution_clock::now();
    QueryResult res;

    if (catalog.exists(ast.table_name)) {
        res.success = false;
        res.error_message = "Table already exists: " + ast.table_name;
        Logger::get_instance().log_error(sql, res.error_message);
        return res;
    }

    auto new_table = std::make_unique<Table>(ast.table_name, ast.columns, ast.pk_columns);
    Table *t_ptr = new_table.get();
    catalog.addTable(std::move(new_table));

    index_mgr.build_indexes_for_table(*t_ptr);
    storage_mgr.save_catalog_meta(catalog);

    auto end_time = std::chrono::high_resolution_clock::now();
    res.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    res.success = true;
    res.info_message = "Table '" + ast.table_name + "' created successfully.";

    Logger::get_instance().log_query(sql, true, res.execution_time_ms, 0, 0, res.info_message);
    return res;
}

QueryResult ExecutionEngine::execute_insert(const AST_Insert &ast, const std::string &sql) {
    auto start_time = std::chrono::high_resolution_clock::now();
    QueryResult res;

    Table *t = catalog.getTable(ast.table_name);
    if (!t) {
        res.success = false;
        res.error_message = "Table not found: " + ast.table_name;
        Logger::get_instance().log_error(sql, res.error_message);
        return res;
    }

    const auto &cols = t->get_columns();
    if (ast.raw_values.size() != cols.size()) {
        res.success = false;
        res.error_message = "Column count mismatch: table expects " + std::to_string(cols.size()) +
                            " values, but got " + std::to_string(ast.raw_values.size());
        Logger::get_instance().log_error(sql, res.error_message);
        return res;
    }

    Row new_row;
    new_row.reserve(cols.size());
    for (size_t i = 0; i < cols.size(); ++i) {
        new_row.push_back(parse_raw_literal(ast.raw_values[i], cols[i]));
    }

    // Primary key duplicate check using index or table logic
    if (t->has_pk()) {
        std::string pk_val = t->build_pk_key_by_row(new_row);
        int existing_idx = index_mgr.lookup_pk(ast.table_name, pk_val);
        if (existing_idx != NOT_FOUND) {
            res.success = false;
            res.error_message = "Duplicate Primary Key violation: key '" + pk_val + "' already exists";
            Logger::get_instance().log_error(sql, res.error_message);
            return res;
        }
    }

    size_t new_row_idx = t->row_count();
    t->insert_row(new_row);
    index_mgr.add_row(*t, new_row, new_row_idx);
    storage_mgr.save_table(*t);

    auto end_time = std::chrono::high_resolution_clock::now();
    res.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    res.success = true;
    res.rows_affected = 1;
    res.info_message = "1 row inserted into '" + ast.table_name + "'.";

    Logger::get_instance().log_query(sql, true, res.execution_time_ms, 0, 1, res.info_message);
    return res;
}

QueryResult ExecutionEngine::execute_select(const AST_Select &ast, const std::string &sql) {
    auto start_time = std::chrono::high_resolution_clock::now();
    QueryResult res;

    Table *t = catalog.getTable(ast.table_name);
    if (!t) {
        res.success = false;
        res.error_message = "Table not found: " + ast.table_name;
        Logger::get_instance().log_error(sql, res.error_message);
        return res;
    }

    res.plan = QueryAnalyzer::analyze_select(*t, ast, index_mgr);

    // Determine projected columns
    std::vector<int> col_indices;
    if (ast.select_list.size() == 1 && ast.select_list[0] == "*") {
        for (size_t i = 0; i < t->get_column_count(); ++i) {
            col_indices.push_back(static_cast<int>(i));
            res.columns.push_back(t->get_column(i).get_name());
        }
    } else {
        for (const auto &col_name : ast.select_list) {
            int idx = t->get_column_index(col_name);
            if (idx == NOT_FOUND) {
                res.success = false;
                res.error_message = "Column not found: " + col_name + " in table " + ast.table_name;
                Logger::get_instance().log_error(sql, res.error_message);
                return res;
            }
            col_indices.push_back(idx);
            res.columns.push_back(col_name);
        }
    }

    // Execute query using Plan strategy
    const auto &rows = t->get_rows();
    std::vector<size_t> candidate_row_indices;

    if (res.plan.strategy == ExecutionStrategy::PRIMARY_KEY_LOOKUP) {
        std::string pk_val = ast.where[0].rhs;
        if (pk_val.size() >= 2 && ((pk_val.front() == '\'' && pk_val.back() == '\'') ||
                                   (pk_val.front() == '"' && pk_val.back() == '"'))) {
            pk_val = pk_val.substr(1, pk_val.size() - 2);
        }
        int idx = index_mgr.lookup_pk(ast.table_name, pk_val);
        if (idx != NOT_FOUND) candidate_row_indices.push_back(static_cast<size_t>(idx));
        res.rows_scanned = 1;
    } else if (res.plan.strategy == ExecutionStrategy::SECONDARY_INDEX_LOOKUP) {
        std::string sec_val = ast.where[0].rhs;
        if (sec_val.size() >= 2 && ((sec_val.front() == '\'' && sec_val.back() == '\'') ||
                                    (sec_val.front() == '"' && sec_val.back() == '"'))) {
            sec_val = sec_val.substr(1, sec_val.size() - 2);
        }
        candidate_row_indices = index_mgr.lookup_secondary(ast.table_name, ast.where[0].lhs, sec_val);
        res.rows_scanned = candidate_row_indices.size();
    } else {
        // Full table scan
        for (size_t i = 0; i < rows.size(); ++i) {
            candidate_row_indices.push_back(i);
        }
        res.rows_scanned = rows.size();
    }

    // Evaluate WHERE conditions on candidate rows
    for (size_t row_idx : candidate_row_indices) {
        if (row_idx >= rows.size()) continue;
        const auto &row = rows[row_idx];
        bool matches = true;
        for (const auto &cond : ast.where) {
            int c_idx = t->get_column_index(cond.lhs);
            if (c_idx == NOT_FOUND || !cond.evaluate(row[c_idx])) {
                matches = false;
                break;
            }
        }
        if (matches) {
            std::vector<std::string> row_str;
            row_str.reserve(col_indices.size());
            for (int c_idx : col_indices) {
                row_str.push_back(row[c_idx].to_string());
            }
            res.rows.push_back(std::move(row_str));
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    res.plan.execution_time_ms = res.execution_time_ms;
    res.plan.rows_returned = res.rows.size();
    res.rows_affected = res.rows.size();
    res.success = true;

    Logger::get_instance().log_query(sql, true, res.execution_time_ms, res.rows_scanned, res.rows_affected,
                                     res.plan.strategy_to_string());
    return res;
}

QueryResult ExecutionEngine::execute_update(const AST_Update &ast, const std::string &sql) {
    auto start_time = std::chrono::high_resolution_clock::now();
    QueryResult res;

    Table *t = catalog.getTable(ast.table_name);
    if (!t) {
        res.success = false;
        res.error_message = "Table not found: " + ast.table_name;
        Logger::get_instance().log_error(sql, res.error_message);
        return res;
    }

    res.plan = QueryAnalyzer::analyze_update(*t, ast, index_mgr);
    auto &rows = t->get_rows();
    size_t updated_count = 0;

    for (size_t i = 0; i < rows.size(); ++i) {
        bool matches = true;
        for (const auto &cond : ast.where) {
            int c_idx = t->get_column_index(cond.lhs);
            if (c_idx == NOT_FOUND || !cond.evaluate(rows[i][c_idx])) {
                matches = false;
                break;
            }
        }
        if (matches) {
            Row new_row = rows[i];
            for (const auto &[col_name, raw_val] : ast.sets) {
                int c_idx = t->get_column_index(col_name);
                if (c_idx != NOT_FOUND) {
                    new_row[c_idx] = parse_raw_literal(raw_val, t->get_column(c_idx));
                }
            }
            t->update_row_at_index(i, new_row);
            updated_count++;
        }
    }

    if (updated_count > 0) {
        index_mgr.rebuild_indexes(*t);
        storage_mgr.save_table(*t);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    res.success = true;
    res.rows_scanned = rows.size();
    res.rows_affected = updated_count;
    res.info_message = std::to_string(updated_count) + " row(s) updated in '" + ast.table_name + "'.";

    Logger::get_instance().log_query(sql, true, res.execution_time_ms, res.rows_scanned, res.rows_affected, res.info_message);
    return res;
}

QueryResult ExecutionEngine::execute_delete(const AST_Delete &ast, const std::string &sql) {
    auto start_time = std::chrono::high_resolution_clock::now();
    QueryResult res;

    Table *t = catalog.getTable(ast.table_name);
    if (!t) {
        res.success = false;
        res.error_message = "Table not found: " + ast.table_name;
        Logger::get_instance().log_error(sql, res.error_message);
        return res;
    }

    res.plan = QueryAnalyzer::analyze_delete(*t, ast, index_mgr);
    auto &rows = t->get_rows();
    size_t deleted_count = 0;

    for (int i = static_cast<int>(rows.size()) - 1; i >= 0; --i) {
        bool matches = true;
        for (const auto &cond : ast.where) {
            int c_idx = t->get_column_index(cond.lhs);
            if (c_idx == NOT_FOUND || !cond.evaluate(rows[i][c_idx])) {
                matches = false;
                break;
            }
        }
        if (matches) {
            t->delete_row_at_index(static_cast<size_t>(i));
            deleted_count++;
        }
    }

    if (deleted_count > 0) {
        index_mgr.rebuild_indexes(*t);
        storage_mgr.save_table(*t);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.execution_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    res.success = true;
    res.rows_scanned = rows.size() + deleted_count;
    res.rows_affected = deleted_count;
    res.info_message = std::to_string(deleted_count) + " row(s) deleted from '" + ast.table_name + "'.";

    Logger::get_instance().log_query(sql, true, res.execution_time_ms, res.rows_scanned, res.rows_affected, res.info_message);
    return res;
}

QueryResult ExecutionEngine::execute_explain(const AST_Explain &ast, const std::string &sql) {
    QueryResult inner_res = execute_query(ast.inner_sql);
    QueryResult res;
    res.success = inner_res.success;
    res.error_message = inner_res.error_message;
    res.execution_time_ms = inner_res.execution_time_ms;
    res.plan = inner_res.plan;
    res.info_message = inner_res.plan.to_formatted_string();
    return res;
}

QueryResult ExecutionEngine::execute_nlp(const AST_Nlp &ast, const std::string &sql) {
    std::string converted_sql = NlpToSql::convert_to_sql(ast.natural_language_query);
    QueryResult res = execute_query(converted_sql);
    if (res.success) {
        res.info_message += " [AI Transliterated SQL: " + converted_sql + "]";
    }
    return res;
}

} // namespace filedb
