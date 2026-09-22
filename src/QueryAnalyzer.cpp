#include "../include/db/QueryAnalyzer.hpp"

namespace filedb {

ExecutionPlan QueryAnalyzer::analyze_select(const Table &table,
                                            const AST_Select &select_ast,
                                            const IndexManager &index_mgr) {
    ExecutionPlan plan;
    plan.table_name = table.get_name();
    plan.total_table_rows = table.row_count();
    plan.indexing_enabled = index_mgr.is_indexing_enabled();

    if (!select_ast.where.empty()) {
        std::stringstream ss;
        for (size_t i = 0; i < select_ast.where.size(); ++i) {
            const auto &cond = select_ast.where[i];
            ss << cond.lhs << " " << cond.op << " " << cond.rhs;
            if (i + 1 < select_ast.where.size()) ss << " AND ";
        }
        plan.filter_condition = ss.str();
    }

    if (!index_mgr.is_indexing_enabled()) {
        plan.strategy = ExecutionStrategy::FULL_TABLE_SCAN;
        plan.index_used = "";
        plan.rows_scanned = table.row_count();
        return plan;
    }

    // Check if WHERE clause matches Primary Key
    if (table.has_pk() && select_ast.where.size() == 1) {
        const auto &cond = select_ast.where[0];
        const auto &pk_cols = table.get_pk_indices();
        if (pk_cols.size() == 1 && (cond.op == "=" || cond.op == "==")) {
            const std::string &pk_col_name = table.get_column(pk_cols[0]).get_name();
            if (cond.lhs == pk_col_name) {
                plan.strategy = ExecutionStrategy::PRIMARY_KEY_LOOKUP;
                plan.index_used = "PRIMARY_KEY (" + pk_col_name + ")";
                plan.rows_scanned = 1;
                return plan;
            }
        }
    }

    // Check secondary index
    if (select_ast.where.size() == 1) {
        const auto &cond = select_ast.where[0];
        if ((cond.op == "=" || cond.op == "==") &&
            index_mgr.has_secondary_index(table.get_name(), cond.lhs)) {
            plan.strategy = ExecutionStrategy::SECONDARY_INDEX_LOOKUP;
            plan.index_used = "HASH_INDEX (" + cond.lhs + ")";
            plan.rows_scanned = index_mgr.lookup_secondary(table.get_name(), cond.lhs, cond.rhs).size();
            return plan;
        }
    }

    plan.strategy = ExecutionStrategy::FULL_TABLE_SCAN;
    plan.rows_scanned = table.row_count();
    return plan;
}

ExecutionPlan QueryAnalyzer::analyze_update(const Table &table,
                                            const AST_Update &update_ast,
                                            const IndexManager &index_mgr) {
    AST_Select dummy_select;
    dummy_select.table_name = update_ast.table_name;
    dummy_select.where = update_ast.where;
    return analyze_select(table, dummy_select, index_mgr);
}

ExecutionPlan QueryAnalyzer::analyze_delete(const Table &table,
                                            const AST_Delete &delete_ast,
                                            const IndexManager &index_mgr) {
    AST_Select dummy_select;
    dummy_select.table_name = delete_ast.table_name;
    dummy_select.where = delete_ast.where;
    return analyze_select(table, dummy_select, index_mgr);
}

} // namespace filedb
