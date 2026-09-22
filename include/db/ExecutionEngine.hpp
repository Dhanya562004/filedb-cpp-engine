#ifndef FILEDB_EXECUTION_ENGINE_HPP
#define FILEDB_EXECUTION_ENGINE_HPP

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "Models.hpp"
#include "StorageManager.hpp"
#include "IndexManager.hpp"
#include "QueryAnalyzer.hpp"
#include "Logger.hpp"

namespace filedb {

struct QueryResult {
    bool success{false};
    std::string error_message;
    std::string info_message;
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> rows;
    double execution_time_ms{0.0};
    size_t rows_scanned{0};
    size_t rows_affected{0};
    ExecutionPlan plan;

    std::string to_cli_table() const;
    std::string to_json() const;
};

class ExecutionEngine {
private:
    Catalog catalog;
    StorageManager storage_mgr;
    IndexManager index_mgr;

public:
    ExecutionEngine();

    Catalog &get_catalog() { return catalog; }
    IndexManager &get_index_manager() { return index_mgr; }
    StorageManager &get_storage_manager() { return storage_mgr; }

    QueryResult execute_query(const std::string &sql);
    QueryResult execute_ast(const AST &ast, const std::string &original_sql);

    void set_indexing_enabled(bool enabled) { index_mgr.set_indexing_enabled(enabled); }
    bool is_indexing_enabled() const { return index_mgr.is_indexing_enabled(); }

private:
    QueryResult execute_create(const AST_Create &ast, const std::string &sql);
    QueryResult execute_insert(const AST_Insert &ast, const std::string &sql);
    QueryResult execute_select(const AST_Select &ast, const std::string &sql);
    QueryResult execute_update(const AST_Update &ast, const std::string &sql);
    QueryResult execute_delete(const AST_Delete &ast, const std::string &sql);
    QueryResult execute_explain(const AST_Explain &ast, const std::string &sql);
    QueryResult execute_nlp(const AST_Nlp &ast, const std::string &sql);

    Value parse_raw_literal(const std::string &raw, const Column &col);
};

} // namespace filedb

#endif // FILEDB_EXECUTION_ENGINE_HPP
