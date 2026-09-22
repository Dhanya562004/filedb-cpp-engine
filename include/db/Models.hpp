#ifndef FILEDB_MODELS_HPP
#define FILEDB_MODELS_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <variant>
#include <iomanip>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>

namespace filedb {

class NullType {
public:
    NullType() = default;
    bool operator==(const NullType&) const { return true; }
    bool operator<(const NullType&) const { return false; }
};

class Date {
private:
    int year{0};
    int month{0};
    int day{0};

public:
    Date() = default;
    Date(int y, int m, int d) : year(y), month(m), day(d) {}

    int get_year() const { return year; }
    int get_month() const { return month; }
    int get_day() const { return day; }

    bool operator==(const Date &other) const {
        return year == other.year && month == other.month && day == other.day;
    }

    bool operator<(const Date &other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }

    std::string to_string() const {
        std::ostringstream ss;
        ss << std::setw(4) << std::setfill('0') << year << '-'
           << std::setw(2) << std::setfill('0') << month << '-'
           << std::setw(2) << std::setfill('0') << day;
        return ss.str();
    }
};

using Int = int;
using Double = double;
using Char = char;
using Text = std::string;
using Variant = std::variant<NullType, Int, Double, Char, Date, Text>;

constexpr int NOT_FOUND = -1;

class Value {
private:
    Variant data;

public:
    Value() : data(NullType{}) {}
    Value(Int value) : data(value) {}
    Value(Double value) : data(value) {}
    Value(Char value) : data(value) {}
    Value(const Date &value) : data(value) {}
    Value(Text value) : data(std::move(value)) {}
    Value(const char *value) : data(Text(value)) {}

    const Variant &raw() const { return data; }
    bool is_null() const { return std::holds_alternative<NullType>(data); }

    Int get_int() const {
        if (std::holds_alternative<Int>(data)) return std::get<Int>(data);
        if (std::holds_alternative<Double>(data)) return static_cast<Int>(std::get<Double>(data));
        if (std::holds_alternative<Text>(data)) {
            try { return std::stoi(std::get<Text>(data)); } catch (...) { return 0; }
        }
        return 0;
    }

    Double get_double() const {
        if (std::holds_alternative<Double>(data)) return std::get<Double>(data);
        if (std::holds_alternative<Int>(data)) return static_cast<Double>(std::get<Int>(data));
        if (std::holds_alternative<Text>(data)) {
            try { return std::stod(std::get<Text>(data)); } catch (...) { return 0.0; }
        }
        return 0.0;
    }

    Text to_string() const {
        return std::visit([](const auto &val) -> Text {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, NullType>) return "NULL";
            else if constexpr (std::is_same_v<T, Int>) return std::to_string(val);
            else if constexpr (std::is_same_v<T, Double>) {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << val;
                std::string s = ss.str();
                // trim trailing zeroes after decimal point if whole number representation
                return s;
            }
            else if constexpr (std::is_same_v<T, Char>) return Text(1, val);
            else if constexpr (std::is_same_v<T, Date>) return val.to_string();
            else if constexpr (std::is_same_v<T, Text>) return val;
            return "";
        }, data);
    }

    bool operator==(const Value &other) const {
        if (is_null() && other.is_null()) return true;
        if (is_null() || other.is_null()) return false;

        if (std::holds_alternative<Int>(data) && std::holds_alternative<Int>(other.data))
            return std::get<Int>(data) == std::get<Int>(other.data);
        if (std::holds_alternative<Double>(data) && std::holds_alternative<Double>(other.data))
            return std::get<Double>(data) == std::get<Double>(other.data);

        if ((std::holds_alternative<Int>(data) || std::holds_alternative<Double>(data)) &&
            (std::holds_alternative<Int>(other.data) || std::holds_alternative<Double>(other.data))) {
            return get_double() == other.get_double();
        }

        if (std::holds_alternative<Date>(data) && std::holds_alternative<Date>(other.data))
            return std::get<Date>(data) == std::get<Date>(other.data);

        return to_string() == other.to_string();
    }

    bool operator<(const Value &other) const {
        if (is_null() && other.is_null()) return false;
        if (is_null()) return true;
        if (other.is_null()) return false;

        if ((std::holds_alternative<Int>(data) || std::holds_alternative<Double>(data)) &&
            (std::holds_alternative<Int>(other.data) || std::holds_alternative<Double>(other.data))) {
            return get_double() < other.get_double();
        }

        if (std::holds_alternative<Date>(data) && std::holds_alternative<Date>(other.data))
            return std::get<Date>(data) < std::get<Date>(other.data);

        return to_string() < other.to_string();
    }

    bool operator>(const Value &other) const { return other < *this; }
    bool operator<=(const Value &other) const { return !(*this > other); }
    bool operator>=(const Value &other) const { return !(*this < other); }
    bool operator!=(const Value &other) const { return !(*this == other); }
};

class Column {
private:
    Text name;
    Text type{"TEXT"};
    bool is_pk_{false};
    bool is_null_{true};
    int char_length{255};

public:
    Column() = default;
    Column(Text n, Text t, bool pk = false, int len = 255, bool nullable = true)
        : name(std::move(n)), type(std::move(t)), is_pk_(pk), is_null_(nullable), char_length(len) {}

    const Text &get_name() const { return name; }
    const Text &get_type() const { return type; }
    bool is_pk() const { return is_pk_; }
    bool is_null() const { return is_null_; }
    int get_char_length() const { return char_length; }

    void set_name(const Text &n) { name = n; }
    void set_type(const Text &t) { type = t; }
    void set_is_pk(bool pk) { is_pk_ = pk; }
    void set_is_null(bool nullable) { is_null_ = nullable; }
    void set_char_length(int len) { char_length = len; }
};

class Row {
private:
    std::vector<Value> vals;

public:
    Row() = default;
    Row(std::vector<Value> values) : vals(std::move(values)) {}

    const std::vector<Value> &values() const { return vals; }
    std::vector<Value> &values() { return vals; }
    size_t size() const { return vals.size(); }
    const Value &at(size_t idx) const { return vals.at(idx); }
    Value &at(size_t idx) { return vals.at(idx); }
    const Value &operator[](size_t idx) const { return vals[idx]; }
    Value &operator[](size_t idx) { return vals[idx]; }
    void push_back(Value value) { vals.push_back(std::move(value)); }
    void reserve(size_t n) { vals.reserve(n); }
};

class Condition {
public:
    Text lhs;
    Text op;
    Text rhs;

    Condition() = default;
    Condition(Text l, Text o, Text r) : lhs(std::move(l)), op(std::move(o)), rhs(std::move(r)) {}

    bool evaluate(const Value &field_val) const;
};

class Table {
private:
    Text name;
    std::vector<Column> columns;
    std::vector<Row> rows;
    std::vector<int> pk_indices;

public:
    Table() = default;
    Table(const Text &tableName,
          const std::vector<Column> &cols,
          const std::vector<Text> &pkColNames = {});

    const Text &get_name() const { return name; }
    const std::vector<Column> &get_columns() const { return columns; }
    std::vector<Column> &get_columns() { return columns; }
    const std::vector<Row> &get_rows() const { return rows; }
    std::vector<Row> &get_rows() { return rows; }
    const std::vector<int> &get_pk_indices() const { return pk_indices; }
    
    size_t row_count() const { return rows.size(); }
    const Row &row_at(size_t i) const { return rows.at(i); }
    Row &row_at(size_t i) { return rows.at(i); }
    
    bool has_pk() const { return !pk_indices.empty(); }
    size_t get_column_count() const { return columns.size(); }
    
    int get_column_index(const Text &col_name) const {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].get_name() == col_name) return static_cast<int>(i);
        }
        return -1;
    }

    const Column &get_column(size_t idx) const { return columns.at(idx); }
    Column &get_column(size_t idx) { return columns.at(idx); }

    void insert_row(Row row);
    void update_row_at_index(size_t idx, const Row &new_row);
    void delete_row_at_index(size_t idx);
    void clear_rows() { rows.clear(); }
    
    Text build_pk_key_by_row(const Row &row) const;
};

class Catalog {
private:
    std::unordered_map<Text, std::unique_ptr<Table>> tables;

public:
    Catalog() = default;
    
    void addTable(std::unique_ptr<Table> t);
    Table *getTable(const Text &name) const;
    bool exists(const Text &name) const;
    std::vector<Text> get_table_names() const;
};

enum class ASTKind {
    CREATE,
    INSERT,
    SELECT,
    UPDATE,
    _DELETE,
    EXPLAIN,
    NLP,
    HELP,
    UNKNOWN
};

struct AST_Create {
    Text table_name;
    std::vector<Column> columns;
    std::vector<Text> pk_columns;
};

struct AST_Insert {
    Text table_name;
    std::vector<Text> raw_values;
};

struct AST_Select {
    Text table_name;
    std::vector<Text> select_list;
    std::vector<Condition> where;
    std::vector<Text> group_by;
    std::vector<Condition> having;
};

struct AST_Update {
    Text table_name;
    std::vector<std::pair<Text, Text>> sets;
    std::vector<Condition> where;
};

struct AST_Delete {
    Text table_name;
    std::vector<Condition> where;
};

struct AST_Explain {
    std::string inner_sql;
};

struct AST_Nlp {
    std::string natural_language_query;
};

class AST {
public:
    ASTKind kind{ASTKind::UNKNOWN};
    std::variant<AST_Create, AST_Insert, AST_Select, AST_Update, AST_Delete, AST_Explain, AST_Nlp> node;
};

} // namespace filedb

#endif // FILEDB_MODELS_HPP
