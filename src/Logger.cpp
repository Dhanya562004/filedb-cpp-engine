#include "../include/db/Logger.hpp"

namespace filedb {

Logger::Logger() : log_filename("logs.txt") {
    log_stream.open(log_filename, std::ios::app);
    if (log_stream.is_open()) {
        log(LogLevel::INFO, "=== FileDB++ Engine Logging Session Started ===");
    }
}

Logger::~Logger() {
    if (log_stream.is_open()) {
        log(LogLevel::INFO, "=== FileDB++ Engine Logging Session Ended ===");
        log_stream.close();
    }
}

Logger &Logger::get_instance() {
    static Logger instance;
    return instance;
}

void Logger::set_log_file(const std::string &filename) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_stream.is_open()) {
        log_stream.close();
    }
    log_filename = filename;
    log_stream.open(log_filename, std::ios::app);
}

void Logger::set_echo_to_console(bool echo) {
    echo_to_console = echo;
}

std::string Logger::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    std::tm time_info{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&time_info, &in_time_t);
#else
    localtime_r(&in_time_t, &time_info);
#endif
    ss << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "LOG";
    }
}

void Logger::log(LogLevel level, const std::string &message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::string timestamp = get_timestamp();
    std::string entry = "[" + timestamp + "] [" + level_to_string(level) + "] " + message;
    
    if (log_stream.is_open()) {
        log_stream << entry << "\n";
        log_stream.flush();
    }
    if (echo_to_console) {
        std::cout << entry << "\n";
    }
}

void Logger::log_query(const std::string &query,
                       bool success,
                       double duration_ms,
                       size_t rows_scanned,
                       size_t rows_affected,
                       const std::string &details) {
    std::stringstream ss;
    ss << "QUERY: \"" << query << "\" | STATUS: " << (success ? "SUCCESS" : "FAILED")
       << " | TIME: " << std::fixed << std::setprecision(3) << duration_ms << " ms"
       << " | ROWS SCANNED: " << rows_scanned
       << " | ROWS RETURNED/AFFECTED: " << rows_affected;
    if (!details.empty()) {
        ss << " | DETAILS: " << details;
    }
    log(success ? LogLevel::INFO : LogLevel::ERROR, ss.str());
}

void Logger::log_error(const std::string &query, const std::string &error_msg) {
    std::stringstream ss;
    ss << "QUERY FAILED: \"" << query << "\" | ERROR: " << error_msg;
    log(LogLevel::ERROR, ss.str());
}

} // namespace filedb
