#ifndef FILEDB_LOGGER_HPP
#define FILEDB_LOGGER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <mutex>

namespace filedb {

enum class LogLevel {
    INFO,
    WARN,
    ERROR,
    DEBUG
};

class Logger {
private:
    std::string log_filename;
    std::ofstream log_stream;
    std::mutex log_mutex;
    bool echo_to_console{false};

    Logger();
    ~Logger();

public:
    static Logger &get_instance();

    void set_log_file(const std::string &filename);
    void set_echo_to_console(bool echo);

    void log(LogLevel level, const std::string &message);
    void log_query(const std::string &query,
                   bool success,
                   double duration_ms,
                   size_t rows_scanned,
                   size_t rows_affected,
                   const std::string &details = "");

    void log_error(const std::string &query, const std::string &error_msg);

private:
    std::string get_timestamp() const;
    std::string level_to_string(LogLevel level) const;
};

} // namespace filedb

#endif // FILEDB_LOGGER_HPP
