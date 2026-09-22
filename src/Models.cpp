#include "../include/db/Models.hpp"

namespace filedb {

bool Condition::evaluate(const Value &field_val) const {
    Value rhs_val;
    
    // Parse RHS into Value type based on field_val type or best effort
    std::string clean_rhs = rhs;
    if (clean_rhs.size() >= 2 && ((clean_rhs.front() == '\'' && clean_rhs.back() == '\'') ||
                                  (clean_rhs.front() == '"' && clean_rhs.back() == '"'))) {
        clean_rhs = clean_rhs.substr(1, clean_rhs.size() - 2);
    }

    if (std::holds_alternative<Int>(field_val.raw())) {
        try { rhs_val = Value(std::stoi(clean_rhs)); } catch (...) { rhs_val = Value(clean_rhs); }
    } else if (std::holds_alternative<Double>(field_val.raw())) {
        try { rhs_val = Value(std::stod(clean_rhs)); } catch (...) { rhs_val = Value(clean_rhs); }
    } else {
        rhs_val = Value(clean_rhs);
    }

    if (op == "=" || op == "==") {
        return field_val == rhs_val;
    } else if (op == "!=" || op == "<>") {
        return field_val != rhs_val;
    } else if (op == "<") {
        return field_val < rhs_val;
    } else if (op == ">") {
        return field_val > rhs_val;
    } else if (op == "<=") {
        return field_val <= rhs_val;
    } else if (op == ">=") {
        return field_val >= rhs_val;
    }
    return false;
}

Table::Table(const Text &tableName,
             const std::vector<Column> &cols,
             const std::vector<Text> &pkColNames)
    : name(tableName), columns(cols) {
    
    if (!pkColNames.empty()) {
        for (const auto &pn : pkColNames) {
            int found = get_column_index(pn);
            if (found == NOT_FOUND) {
                throw std::runtime_error("PRIMARY KEY column not found: " + pn);
            }
            columns[found].set_is_pk(true);
            pk_indices.push_back(found);
        }
    } else {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].is_pk()) {
                pk_indices.push_back(static_cast<int>(i));
            }
        }
    }
}

Text Table::build_pk_key_by_row(const Row &row) const {
    if (pk_indices.empty()) {
        return "";
    }
    std::string key;
    bool is_first = true;
    for (int idx : pk_indices) {
        if (!is_first) key += "|";
        is_first = false;
        const Value &val = row.at(static_cast<size_t>(idx));
        if (val.is_null()) {
            throw std::runtime_error("Primary Key column value cannot be NULL");
        }
        key += val.to_string();
    }
    return key;
}

void Table::insert_row(Row row) {
    rows.push_back(std::move(row));
}

void Table::update_row_at_index(size_t idx, const Row &new_row) {
    if (idx >= rows.size()) {
        throw std::out_of_range("Table update index out of range");
    }
    rows[idx] = new_row;
}

void Table::delete_row_at_index(size_t idx) {
    if (idx >= rows.size()) {
        throw std::out_of_range("Table delete index out of range");
    }
    rows.erase(rows.begin() + idx);
}

void Catalog::addTable(std::unique_ptr<Table> t) {
    if (!t) throw std::invalid_argument("addTable: null pointer");
    std::string name = t->get_name();
    tables[name] = std::move(t);
}

Table *Catalog::getTable(const Text &name) const {
    auto it = tables.find(name);
    if (it == tables.end()) return nullptr;
    return it->second.get();
}

bool Catalog::exists(const Text &name) const {
    return tables.find(name) != tables.end();
}

std::vector<Text> Catalog::get_table_names() const {
    std::vector<Text> names;
    names.reserve(tables.size());
    for (const auto &[name, table] : tables) {
        names.push_back(name);
    }
    return names;
}

} // namespace filedb
