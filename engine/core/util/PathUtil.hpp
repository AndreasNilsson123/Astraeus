#ifndef ASTRAEUS_CORE_UTIL_PATH_UTIL_HPP
#define ASTRAEUS_CORE_UTIL_PATH_UTIL_HPP

/**
 * Path utility functions for file system operations.
 */

#include <string>
#include "StringUtil.hpp"

namespace astraeus {
namespace path_util {

/**
 * Get the file extension from a path (including the dot).
 */
inline std::string get_extension(const std::string& path) {
    size_t dot_pos = path.find_last_of('.');
    size_t sep_pos = path.find_last_of("/\\");
    
    // Check if dot is after last separator (or no separator exists)
    if (dot_pos != std::string::npos && 
        (sep_pos == std::string::npos || dot_pos > sep_pos)) {
        return path.substr(dot_pos);
    }
    return "";
}

/**
 * Get the filename from a path (without directory).
 */
inline std::string get_filename(const std::string& path) {
    size_t sep_pos = path.find_last_of("/\\");
    if (sep_pos != std::string::npos) {
        return path.substr(sep_pos + 1);
    }
    return path;
}

/**
 * Get the directory from a path (without filename).
 */
inline std::string get_directory(const std::string& path) {
    size_t sep_pos = path.find_last_of("/\\");
    if (sep_pos != std::string::npos) {
        return path.substr(0, sep_pos);
    }
    return "";
}

/**
 * Join two path components.
 */
inline std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    
    char last = a.back();
    if (last == '/' || last == '\\') {
        return a + b;
    }
    return a + "/" + b;
}

/**
 * Normalize path separators to forward slashes.
 */
inline std::string normalize(const std::string& path) {
    return string_util::replace_all(path, "\\", "/");
}

} // namespace path_util
} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_PATH_UTIL_HPP
