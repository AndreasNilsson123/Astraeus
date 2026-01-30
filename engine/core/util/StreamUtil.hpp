#ifndef ASTRAEUS_CORE_UTIL_STREAM_UTIL_HPP
#define ASTRAEUS_CORE_UTIL_STREAM_UTIL_HPP

/**
 * Stream utility functions for formatted output.
 */

#include <ostream>
#include <iomanip>
#include "Math.hpp"

namespace astraeus {
namespace stream_util {

/**
 * Format floating point value with fixed precision.
 */
template <typename T>
struct Fixed {
    T value;
    int precision;

    Fixed(T v, int p = 2) : value(v), precision(p) {}
};

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const Fixed<T>& f) {
    auto flags = os.flags();
    os << std::fixed << std::setprecision(f.precision) << f.value;
    os.flags(flags);
    return os;
}

/**
 * Format floating point value with scientific notation.
 */
template <typename T>
struct Scientific {
    T value;
    int precision;

    Scientific(T v, int p = 2) : value(v), precision(p) {}
};

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const Scientific<T>& s) {
    auto flags = os.flags();
    os << std::scientific << std::setprecision(s.precision) << s.value;
    os.flags(flags);
    return os;
}

/**
 * Format value with padding.
 */
template <typename T>
struct Padded {
    T value;
    int width;
    char fill;

    Padded(T v, int w, char f = ' ') : value(v), width(w), fill(f) {}
};

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const Padded<T>& p) {
    auto flags = os.flags();
    auto old_fill = os.fill();
    os << std::setw(p.width) << std::setfill(p.fill) << p.value;
    os.fill(old_fill);
    os.flags(flags);
    return os;
}

/**
 * Format boolean as "true"/"false" instead of 1/0.
 */
struct BoolAlpha {
    bool value;
    explicit BoolAlpha(bool v) : value(v) {}
};

inline std::ostream& operator<<(std::ostream& os, const BoolAlpha& b) {
    return os << (b.value ? "true" : "false");
}

} // namespace stream_util
} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_STREAM_UTIL_HPP
