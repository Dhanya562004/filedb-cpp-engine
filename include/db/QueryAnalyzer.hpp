#ifndef FILEDB_QUERY_ANALYZER_HPP
#define FILEDB_QUERY_ANALYZER_HPP

#include <string>
#include <vector>
#include <sstream>
#include "Models.hpp"
#include "IndexManager.hpp"

namespace filedb {

enum class ExecutionStrategy {
    PRIMARY_KEY_LOOKUP,
    SECONDARY_INDEX_LOOKUP,
    FULL_TABLE_SCAN
};

struct ExecutionPlan {
    std::string table_name;
    ExecutionStrategy strategy{ExecutionStrategy::FULL_TABLE_SCAN};
    std::string index_used;
    std::string filter_condition;
    size_t total_table_rows{0};
    size_t rows_scanned{0};
    size_t rows_returned{0};
    double execution_time_ms{0.0};
    bool indexing_enabled{true};

    std::string strategy_to_string() const {
        switch (strategy) {
            case ExecutionStrategy::PRIMARY_KEY_LOOKUP: return "PRIMARY_KEY_HASH_INDEX";
            case ExecutionStrategy::SECONDARY_INDEX_LOOKUP: return "SECONDARY_HASH_INDEX";
            case ExecutionStrategy::FULL_TABLE_SCAN: return "FULL_TABLE_SCAN";
            default: return "UNKNOWN";
        }
    }

    std::string to_formatted_string() const {
        std::stringstream ss;
        ss << "================ QUERY EXECUTION PLAN ================\n"
           << " Target Table    : " << table_name << "\n"
           << " Access Strategy : " << strategy_to_string() << "\n"
           << " Index Used      : " << (index_used.empty() ? "None (Sequential Scan)" : index_used) << "\n"
           << " Filter          : " << (filter_condition.empty() ? "None" : filter_condition) << "\n"
           << " Total Rows      : " << total_table_rows << "\n"
           << " Rows Scanned    : " << rows_scanned << "\n"
           << " Rows Returned   : " << rows_returned << "\n"
           << " Execution Time  : " << execution_time_ms << " ms\n"
           << " Indexing Mode   : " << (indexing_enabled ? "ENABLED" : "DISABLED") << "\n"
           << "======================================================";
        return ss.str();
    }
};

class QueryAnalyzer {
public:
    static ExecutionPlan analyze_select(const Table &table,
                                        const AST_Select &select_ast,
                                        const IndexManager &index_mgr);

    static ExecutionPlan analyze_update(const Table &table,
                                        const AST_Update &update_ast,
                                        const IndexManager &index_mgr);

    static ExecutionPlan analyze_delete(const Table &table,
                                        const AST_Delete &delete_ast,
                                        const IndexManager &index_mgr);
};

} // namespace filedb

#endif // FILEDB_QUERY_ANALYZER_HPP
