#ifndef FILEDB_STORAGE_MANAGER_HPP
#define FILEDB_STORAGE_MANAGER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include "Models.hpp"

namespace filedb {

class StorageManager {
private:
    std::string data_directory;
    static constexpr size_t BUFFER_SIZE = 64 * 1024; // 64 KB buffer for high-performance stream I/O

public:
    explicit StorageManager(std::string data_dir = "data");

    void ensure_data_dir_exists() const;

    bool save_table(const Table &table) const;
    bool load_table(Table &table) const;
    bool load_all_tables(Catalog &catalog) const;

    bool save_catalog_meta(const Catalog &catalog) const;
    bool load_catalog_meta(Catalog &catalog) const;

private:
    std::string get_table_file_path(const std::string &table_name) const;
    std::string get_catalog_meta_path() const;

    Value parse_value_for_column(const std::string &token, const Column &col) const;
};

} // namespace filedb

#endif // FILEDB_STORAGE_MANAGER_HPP
