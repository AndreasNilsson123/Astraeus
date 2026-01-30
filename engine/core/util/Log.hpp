#ifndef ASTRAEUS_CORE_UTIL_LOG_HPP
#define ASTRAEUS_CORE_UTIL_LOG_HPP

/**
 * Simple logging utilities for the Astraeus engine.
 */

#include <iostream>
#include <sstream>
#include <string>

namespace astraeus {
namespace log {

enum class Level {
    Debug,
    Info,
    Warning,
    Error
};

/**
 * Get the current log level.
 */
inline Level& get_level() {
    static Level level = Level::Info;
    return level;
}

/**
 * Set the minimum log level.
 */
inline void set_level(Level level) {
    get_level() = level;
}

/**
 * Internal logging function.
 */
inline void log_message(Level level, const std::string& message) {
    if (level < get_level()) {
        return;
    }

    const char* level_str = "";
    std::ostream* stream = &std::cout;

    switch (level) {
        case Level::Debug:
            level_str = "[DEBUG] ";
            break;
        case Level::Info:
            level_str = "[INFO]  ";
            break;
        case Level::Warning:
            level_str = "[WARN]  ";
            stream = &std::cerr;
            break;
        case Level::Error:
            level_str = "[ERROR] ";
            stream = &std::cerr;
            break;
    }

    *stream << level_str << message << std::endl;
}

/**
 * Log debug message.
 */
inline void debug(const std::string& message) {
    log_message(Level::Debug, message);
}

/**
 * Log info message.
 */
inline void info(const std::string& message) {
    log_message(Level::Info, message);
}

/**
 * Log warning message.
 */
inline void warning(const std::string& message) {
    log_message(Level::Warning, message);
}

/**
 * Log error message.
 */
inline void error(const std::string& message) {
    log_message(Level::Error, message);
}

} // namespace log
} // namespace astraeus

// Convenience macros
#define ASTRAEUS_LOG_DEBUG(msg) ::astraeus::log::debug(msg)
#define ASTRAEUS_LOG_INFO(msg) ::astraeus::log::info(msg)
#define ASTRAEUS_LOG_WARNING(msg) ::astraeus::log::warning(msg)
#define ASTRAEUS_LOG_ERROR(msg) ::astraeus::log::error(msg)

#endif // ASTRAEUS_CORE_UTIL_LOG_HPP
