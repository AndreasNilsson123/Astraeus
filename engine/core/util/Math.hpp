#ifndef ASTRAEUS_CORE_UTIL_MATH_HPP
#define ASTRAEUS_CORE_UTIL_MATH_HPP

/**
 * Astraeus Math Utilities
 * 
 * This header provides a consistent math façade for the Astraeus engine.
 * Goals:
 * 1. Provide stable wrappers around std:: math/algorithm primitives
 * 2. Avoid Windows macro traps (min/max)
 * 3. Centralize future overrides (fast-math, SIMD, platform-specific implementations)
 * 
 * All functions use the astraeus::math namespace to avoid conflicts.
 */

#include <algorithm>
#include <cmath>
#include <type_traits>

namespace astraeus {
namespace math {

// =============================================================================
// Constants
// =============================================================================

template <typename T>
inline constexpr T pi() {
    return static_cast<T>(3.14159265358979323846);
}

template <typename T>
inline constexpr T two_pi() {
    return static_cast<T>(6.28318530717958647692);
}

template <typename T>
inline constexpr T deg_to_rad() {
    return pi<T>() / static_cast<T>(180);
}

template <typename T>
inline constexpr T rad_to_deg() {
    return static_cast<T>(180) / pi<T>();
}

// =============================================================================
// Scalar Functions
// =============================================================================

/**
 * Minimum of two values.
 * Uses (std::min) syntax to defeat Windows min/max macros.
 */
template <typename T>
inline constexpr T min(T a, T b) noexcept {
    return (std::min)(a, b);
}

/**
 * Maximum of two values.
 * Uses (std::max) syntax to defeat Windows min/max macros.
 */
template <typename T>
inline constexpr T max(T a, T b) noexcept {
    return (std::max)(a, b);
}

/**
 * Clamp a value between a minimum and maximum.
 */
template <typename T>
inline constexpr T clamp(T x, T lo, T hi) noexcept {
    return (std::clamp)(x, lo, hi);
}

/**
 * Absolute value.
 */
template <typename T>
inline constexpr T abs(T x) noexcept {
    using std::abs;
    return abs(x);
}

/**
 * Square root.
 */
template <typename T>
inline T sqrt(T x) noexcept {
    using std::sqrt;
    return sqrt(x);
}

/**
 * Reciprocal square root.
 * Currently defaults to 1/sqrt(x), but can be overridden for fast approximations.
 */
template <typename T>
inline T rsqrt(T x) noexcept {
#if defined(ASTRAEUS_USE_FAST_MATH)
    // Future: Platform-specific fast reciprocal sqrt implementations
    // For now, use standard division
    return static_cast<T>(1) / sqrt(x);
#else
    return static_cast<T>(1) / sqrt(x);
#endif
}

/**
 * Floor function (largest integer <= x).
 */
template <typename T>
inline T floor(T x) noexcept {
    using std::floor;
    return floor(x);
}

/**
 * Ceiling function (smallest integer >= x).
 */
template <typename T>
inline T ceil(T x) noexcept {
    using std::ceil;
    return ceil(x);
}

/**
 * Round to nearest integer.
 */
template <typename T>
inline T round(T x) noexcept {
    using std::round;
    return round(x);
}

// =============================================================================
// Trigonometric Functions
// =============================================================================

/**
 * Sine function.
 */
template <typename T>
inline T sin(T x) noexcept {
    using std::sin;
    return sin(x);
}

/**
 * Cosine function.
 */
template <typename T>
inline T cos(T x) noexcept {
    using std::cos;
    return cos(x);
}

/**
 * Tangent function.
 */
template <typename T>
inline T tan(T x) noexcept {
    using std::tan;
    return tan(x);
}

/**
 * Arc sine function.
 */
template <typename T>
inline T asin(T x) noexcept {
    using std::asin;
    return asin(x);
}

/**
 * Arc cosine function.
 */
template <typename T>
inline T acos(T x) noexcept {
    using std::acos;
    return acos(x);
}

/**
 * Arc tangent of y/x, handling quadrants correctly.
 */
template <typename T>
inline T atan2(T y, T x) noexcept {
    using std::atan2;
    return atan2(y, x);
}

// =============================================================================
// Modulo Functions
// =============================================================================

/**
 * Floating-point remainder of x/y.
 * Equivalent to x - n*y where n is truncated division of x/y.
 */
template <typename T>
inline T fmod(T x, T y) noexcept {
    using std::fmod;
    return fmod(x, y);
}

/**
 * IEEE remainder of x/y.
 * Equivalent to x - n*y where n is rounded division of x/y.
 */
template <typename T>
inline T remainder(T x, T y) noexcept {
    using std::remainder;
    return remainder(x, y);
}

// =============================================================================
// Predicates
// =============================================================================

/**
 * Check if value is finite (not infinity or NaN).
 */
template <typename T>
inline bool isfinite(T x) noexcept {
    using std::isfinite;
    return isfinite(x);
}

/**
 * Check if value is NaN (not a number).
 */
template <typename T>
inline bool isnan(T x) noexcept {
    using std::isnan;
    return isnan(x);
}

/**
 * Check sign bit (true for negative values).
 */
template <typename T>
inline bool signbit(T x) noexcept {
    using std::signbit;
    return signbit(x);
}

// =============================================================================
// Power Functions
// =============================================================================

/**
 * Power function (base^exponent).
 */
template <typename T>
inline T pow(T base, T exponent) noexcept {
    using std::pow;
    return pow(base, exponent);
}

/**
 * Exponential function (e^x).
 */
template <typename T>
inline T exp(T x) noexcept {
    using std::exp;
    return exp(x);
}

/**
 * Natural logarithm.
 */
template <typename T>
inline T log(T x) noexcept {
    using std::log;
    return log(x);
}

} // namespace math
} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_MATH_HPP
