#include "../include/db/IndexManager.hpp"
#include <iostream>

namespace filedb {

void IndexManager::build_indexes_for_table(const Table &table) {
    const std::string &table_name = table.get_name();
    clear_table_indexes(table_name);

    const auto &rows = table.get_rows();
    for (size_t i = 0; i < rows.size(); ++i) {
        add_row(table, rows[i], i);
    }
}

void IndexManager::build_secondary_index(const Table &table, const std::string &col_name) {
    const std::string &table_name = table.get_name();
    int col_idx = table.get_column_index(col_name);
    if (col_idx == NOT_FOUND) return;

    auto &col_map = secondary_indices[table_name][col_name];
    col_map.clear();

    const auto &rows = table.get_rows();
    for (size_t i = 0; i < rows.size(); ++i) {
        std::string val_str = rows[i][col_idx].to_string();
        col_map[val_str].push_back(i);
    }
}

void IndexManager::clear_table_indexes(const std::string &table_name) {
    pk_indices.erase(table_name);
    secondary_indices.erase(table_name);
}

void IndexManager::add_row(const Table &table, const Row &row, size_t row_idx) {
    const std::string &table_name = table.get_name();

    if (table.has_pk()) {
        std::string pk_val = table.build_pk_key_by_row(row);
        if (!pk_val.empty()) {
            pk_indices[table_name][pk_val] = row_idx;
        }
    }

    auto sec_it = secondary_indices.find(table_name);
    if (sec_it != secondary_indices.end()) {
        for (auto &[col_name, val_map] : sec_it->second) {
            int col_idx = table.get_column_index(col_name);
            if (col_idx != NOT_FOUND && static_cast<size_t>(col_idx) < row.size()) {
                std::string val_str = row[col_idx].to_string();
                val_map[val_str].push_back(row_idx);
            }
        }
    }
}

void IndexManager::remove_row(const Table &table, const Row &row, size_t row_idx) {
    // Rebuilding is safer and faster after deletes/updates to maintain row index consistency
    rebuild_indexes(table);
}

void IndexManager::rebuild_indexes(const Table &table) {
    build_indexes_for_table(table);
}

int IndexManager::lookup_pk(const std::string &table_name, const std::string &pk_value) const {
    if (!indexing_enabled) return NOT_FOUND;

    auto tbl_it = pk_indices.find(table_name);
    if (tbl_it == pk_indices.end()) return NOT_FOUND;

    auto pk_it = tbl_it->second.find(pk_value);
    if (pk_it == tbl_it->second.end()) return NOT_FOUND;

    return static_cast<int>(pk_it->second);
}

std::vector<size_t> IndexManager::lookup_secondary(const std::string &table_name,
                                                     const std::string &col_name,
                                                     const std::string &value) const {
    if (!indexing_enabled) return {};

    auto tbl_it = secondary_indices.find(table_name);
    if (tbl_it == secondary_indices.end()) return {};

    auto col_it = tbl_it->second.find(col_name);
    if (col_it == tbl_it->second.end()) return {};

    auto val_it = col_it->second.find(value);
    if (val_it == col_it->second.end()) return {};

    return val_it->second;
}

bool IndexManager::has_pk_index(const std::string &table_name) const {
    return indexing_enabled && (pk_indices.find(table_name) != pk_indices.end());
}

bool IndexManager::has_secondary_index(const std::string &table_name, const std::string &col_name) const {
    if (!indexing_enabled) return false;
    auto tbl_it = secondary_indices.find(table_name);
    if (tbl_it == secondary_indices.end()) return false;
    return tbl_it->second.find(col_name) != tbl_it->second.end();
}

} // namespace filedb
