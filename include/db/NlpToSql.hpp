#ifndef FILEDB_NLP_TO_SQL_HPP
#define FILEDB_NLP_TO_SQL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <regex>

namespace filedb {

class NlpToSql {
public:
    static std::string convert_to_sql(const std::string &nl_query);
    static bool is_natural_language(const std::string &input_str);

private:
    static std::string trim(const std::string &str);
    static std::string to_lower(const std::string &str);
};

} // namespace filedb

#endif // FILEDB_NLP_TO_SQL_HPP
