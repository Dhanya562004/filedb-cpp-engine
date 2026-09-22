#include "../include/db/StorageManager.hpp"
#include "../include/db/Logger.hpp"
#include <sstream>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0777)
#endif

namespace filedb {

StorageManager::StorageManager(std::string data_dir) : data_directory(std::move(data_dir)) {
    ensure_data_dir_exists();
}

void StorageManager::ensure_data_dir_exists() const {
    MKDIR(data_directory.c_str());
}

std::string StorageManager::get_table_file_path(const std::string &table_name) const {
    return data_directory + "/" + table_name + ".csv";
}

std::string StorageManager::get_catalog_meta_path() const {
    return data_directory + "/catalog.meta";
}

Value StorageManager::parse_value_for_column(const std::string &token, const Column &col) const {
    if (token == "NULL" || token.empty()) {
        return Value();
    }
    std::string type = col.get_type();
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);

    if (type == "INT" || type == "INTEGER") {
        try { return Value(std::stoi(token)); } catch (...) { return Value(0); }
    } else if (type == "DOUBLE" || type == "FLOAT" || type == "DECIMAL") {
        try { return Value(std::stod(token)); } catch (...) { return Value(0.0); }
    } else if (type == "CHAR" && token.size() == 1) {
        return Value(token[0]);
    } else if (type == "DATE") {
        int y = 0, m = 0, d = 0;
        if (sscanf(token.c_str(), "%d-%d-%d", &y, &m, &d) == 3) {
            return Value(Date(y, m, d));
        }
        return Value(token);
    }
    return Value(token);
}

bool StorageManager::save_table(const Table &table) const {
    ensure_data_dir_exists();
    std::string filepath = get_table_file_path(table.get_name());
    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        Logger::get_instance().log(LogLevel::ERROR, "Failed to open file for writing table: " + filepath);
        return false;
    }

    // Apply high performance stream buffer
    std::vector<char> buffer(BUFFER_SIZE);
    file.rdbuf()->pubsetbuf(buffer.data(), BUFFER_SIZE);

    // Write header line (Column names)
    const auto &cols = table.get_columns();
    for (size_t i = 0; i < cols.size(); ++i) {
        file << cols[i].get_name() << (i + 1 < cols.size() ? "," : "\n");
    }

    // Write row lines
    const auto &rows = table.get_rows();
    for (const auto &row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i].to_string() << (i + 1 < row.size() ? "," : "\n");
        }
    }

    file.flush();
    return true;
}

bool StorageManager::load_table(Table &table) const {
    std::string filepath = get_table_file_path(table.get_name());
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        return false;
    }

    // Apply high performance stream buffer
    std::vector<char> buffer(BUFFER_SIZE);
    file.rdbuf()->pubsetbuf(buffer.data(), BUFFER_SIZE);

    std::string line;
    // Skip header line
    if (!std::getline(file, line)) {
        return false;
    }

    table.clear_rows();
    const auto &cols = table.get_columns();

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        Row row;
        row.reserve(cols.size());

        size_t col_idx = 0;
        while (std::getline(ss, token, ',') && col_idx < cols.size()) {
            row.push_back(parse_value_for_column(token, cols[col_idx]));
            col_idx++;
        }

        if (row.size() == cols.size()) {
            table.insert_row(std::move(row));
        }
    }
    return true;
}

bool StorageManager::save_catalog_meta(const Catalog &catalog) const {
    ensure_data_dir_exists();
    std::string filepath = get_catalog_meta_path();
    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return false;

    auto table_names = catalog.get_table_names();
    for (const auto &name : table_names) {
        Table *t = catalog.getTable(name);
        if (!t) continue;

        file << "TABLE:" << t->get_name() << "\n";
        const auto &cols = t->get_columns();
        for (const auto &col : cols) {
            file << "COL:" << col.get_name() << ":" << col.get_type()
                 << ":" << (col.is_pk() ? "1" : "0")
                 << ":" << col.get_char_length() << "\n";
        }
        file << "END_TABLE\n";

        // Also save table rows to its CSV
        save_table(*t);
    }
    return true;
}

bool StorageManager::load_catalog_meta(Catalog &catalog) const {
    std::string filepath = get_catalog_meta_path();
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string line;
    std::string current_table_name;
    std::vector<Column> current_cols;

    while (std::getline(file, line)) {
        if (line.rfind("TABLE:", 0) == 0) {
            current_table_name = line.substr(6);
            current_cols.clear();
        } else if (line.rfind("COL:", 0) == 0) {
            std::stringstream ss(line.substr(4));
            std::string cname, ctype, cpk_str, clen_str;
            if (std::getline(ss, cname, ':') && std::getline(ss, ctype, ':') &&
                std::getline(ss, cpk_str, ':') && std::getline(ss, clen_str, ':')) {
                bool is_pk = (cpk_str == "1");
                int len = 255;
                try { len = std::stoi(clen_str); } catch (...) {}
                current_cols.emplace_back(cname, ctype, is_pk, len);
            }
        } else if (line == "END_TABLE") {
            if (!current_table_name.empty()) {
                auto new_table = std::make_unique<Table>(current_table_name, current_cols);
                load_table(*new_table);
                catalog.addTable(std::move(new_table));
            }
        }
    }
    return true;
}

bool StorageManager::load_all_tables(Catalog &catalog) const {
    return load_catalog_meta(catalog);
}

} // namespace filedb
