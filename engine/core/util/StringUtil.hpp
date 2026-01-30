#ifndef ASTRAEUS_CORE_UTIL_STRING_UTIL_HPP
#define ASTRAEUS_CORE_UTIL_STRING_UTIL_HPP

/**
 * String utility functions.
 */

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace astraeus {
namespace string_util {

/**
 * Trim whitespace from the left side of a string.
 */
inline std::string trim_left(const std::string& str) {
    auto it = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return std::string(it, str.end());
}

/**
 * Trim whitespace from the right side of a string.
 */
inline std::string trim_right(const std::string& str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return std::string(str.begin(), it.base());
}

/**
 * Trim whitespace from both sides of a string.
 */
inline std::string trim(const std::string& str) {
    return trim_left(trim_right(str));
}

/**
 * Convert string to lowercase.
 */
inline std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * Convert string to uppercase.
 */
inline std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return result;
}

/**
 * Split a string by delimiter.
 */
inline std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    for (char ch : str) {
        if (ch == delimiter) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += ch;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * Check if a string starts with a prefix.
 */
inline bool starts_with(const std::string& str, const std::string& prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }
    return str.compare(0, prefix.size(), prefix) == 0;
}

/**
 * Check if a string ends with a suffix.
 */
inline bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) {
        return false;
    }
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * Replace all occurrences of a substring.
 */
inline std::string replace_all(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

} // namespace string_util
} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_STRING_UTIL_HPP
