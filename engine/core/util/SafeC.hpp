#ifndef ASTRAEUS_SAFE_C_HPP
#define ASTRAEUS_SAFE_C_HPP

#include <cstddef>
#include <cstring>
#include <cstdio>

/**
 * Cross-platform safe C utility functions.
 * 
 * Provides safe alternatives to deprecated C string/parsing functions
 * without requiring global warning suppressions.
 * 
 * Design:
 * - Always null-terminates strings
 * - Uses platform-native secure functions when available
 * - Minimal overhead, header-only
 */

namespace astraeus {
namespace util {

/**
 * Safe string copy that always null-terminates.
 * 
 * @param dst Destination buffer
 * @param dst_size Size of destination buffer (must be > 0)
 * @param src Source string (must be null-terminated)
 */
inline void str_copy(char* dst, std::size_t dst_size, const char* src) {
    if (!dst || dst_size == 0 || !src) {
        return;
    }
    
#if defined(_MSC_VER)
    // Use Microsoft's secure version
    strncpy_s(dst, dst_size, src, _TRUNCATE);
#else
    // Manual safe copy for other platforms
    std::size_t i = 0;
    while (i < dst_size - 1 && src[i] != '\0') {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
#endif
}

/**
 * Safe integer parsing from string.
 * Parses format "x y z" into three integers.
 * 
 * @param s Input string
 * @param a Output: first integer
 * @param b Output: second integer
 * @param c Output: third integer
 * @return Number of successfully parsed integers (0-3)
 */
inline int parse_int3(const char* s, int* a, int* b, int* c) {
    if (!s || !a || !b || !c) {
        return 0;
    }
    
    // Simple manual parser to avoid sscanf warnings
    // This handles the "v1 v2 v3" format common in OBJ files
    const char* p = s;
    int values[3] = {0, 0, 0};
    int count = 0;
    
    // Skip leading whitespace
    while (*p == ' ' || *p == '\t') ++p;
    
    for (int i = 0; i < 3 && *p != '\0'; ++i) {
        // Parse sign
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            ++p;
        } else if (*p == '+') {
            ++p;
        }
        
        // Parse digits
        int value = 0;
        bool found_digit = false;
        while (*p >= '0' && *p <= '9') {
            value = value * 10 + (*p - '0');
            ++p;
            found_digit = true;
        }
        
        if (!found_digit) {
            break;
        }
        
        values[i] = sign * value;
        ++count;
        
        // Skip whitespace between numbers
        while (*p == ' ' || *p == '\t') ++p;
    }
    
    *a = values[0];
    *b = values[1];
    *c = values[2];
    
    return count;
}

/**
 * Safe parsing of "v/vt/vn" format (OBJ face vertex format).
 * Handles formats: "v", "v/vt", "v/vt/vn", "v//vn"
 * 
 * @param s Input string
 * @param v Output: vertex index
 * @param vt Output: texcoord index
 * @param vn Output: normal index
 * @return Number of successfully parsed indices (1-3)
 */
inline int parse_obj_vertex(const char* s, int* v, int* vt, int* vn) {
    if (!s || !v || !vt || !vn) {
        return 0;
    }
    
    *v = 0;
    *vt = 0;
    *vn = 0;
    
    const char* p = s;
    int values[3] = {0, 0, 0};
    int count = 0;
    
    // Parse up to 3 integers separated by '/'
    for (int i = 0; i < 3 && *p != '\0' && *p != ' ' && *p != '\t'; ++i) {
        // Parse sign
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            ++p;
        } else if (*p == '+') {
            ++p;
        }
        
        // Parse digits
        int value = 0;
        bool found_digit = false;
        while (*p >= '0' && *p <= '9') {
            value = value * 10 + (*p - '0');
            ++p;
            found_digit = true;
        }
        
        if (found_digit) {
            values[i] = sign * value;
            ++count;
        }
        
        // Check for separator
        if (*p == '/') {
            ++p;
        } else {
            break;
        }
    }
    
    *v = values[0];
    *vt = values[1];
    *vn = values[2];
    
    return count;
}

} // namespace util
} // namespace astraeus

#endif // ASTRAEUS_SAFE_C_HPP
