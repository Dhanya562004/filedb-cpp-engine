#ifndef FILEDB_INDEX_MANAGER_HPP
#define FILEDB_INDEX_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Models.hpp"

namespace filedb {

class IndexManager {
private:
    // table_name -> (pk_value_string -> row_index)
    std::unordered_map<std::string, std::unordered_map<std::string, size_t>> pk_indices;

    // table_name -> (col_name -> (val_string -> vector of row_indices))
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::vector<size_t>>>> secondary_indices;

    bool indexing_enabled{true};

public:
    IndexManager() = default;

    void set_indexing_enabled(bool enabled) { indexing_enabled = enabled; }
    bool is_indexing_enabled() const { return indexing_enabled; }

    void build_indexes_for_table(const Table &table);
    void build_secondary_index(const Table &table, const std::string &col_name);

    void add_row(const Table &table, const Row &row, size_t row_idx);
    void remove_row(const Table &table, const Row &row, size_t row_idx);
    void rebuild_indexes(const Table &table);

    int lookup_pk(const std::string &table_name, const std::string &pk_value) const;
    std::vector<size_t> lookup_secondary(const std::string &table_name,
                                           const std::string &col_name,
                                           const std::string &value) const;

    bool has_pk_index(const std::string &table_name) const;
    bool has_secondary_index(const std::string &table_name, const std::string &col_name) const;

    void clear_table_indexes(const std::string &table_name);
};

} // namespace filedb

#endif // FILEDB_INDEX_MANAGER_HPP
