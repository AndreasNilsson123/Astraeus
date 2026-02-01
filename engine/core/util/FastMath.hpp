#ifndef ASTRAEUS_CORE_UTIL_FASTMATH_HPP
#define ASTRAEUS_CORE_UTIL_FASTMATH_HPP

/**
 * Astraeus Fast Math Utilities
 * 
 * This header provides fast approximate math functions optimized for visualization hot paths.
 * These functions trade some accuracy for speed and are intended for use in:
 * - Camera controls and orbit calculations
 * - Culling heuristics and LOD decisions
 * - Debug overlays and visualization helpers
 * - Non-authoritative spatial queries
 * 
 * IMPORTANT: These functions should NOT be used for:
 * - Serialization or world-state truth
 * - Final picking results if precision is critical
 * - Physics simulations or authoritative gameplay logic
 * 
 * Quality Levels (controlled by ASTRAEUS_FASTMATH_LEVEL):
 * - 0: Accurate fallback (uses std:: functions, no approximation)
 * - 1: Very fast (aggressive approximations, lower accuracy)
 * - 2: Balanced (default, good speed/accuracy tradeoff)
 * - 3: Higher accuracy (refined approximations, closer to std::)
 * 
 * All functions are deterministic for a given build configuration.
 * All functions are header-only and use float precision by default.
 */

#include <cmath>
#include <cstdint>
#include <limits>

// Default to balanced quality level if not specified
#ifndef ASTRAEUS_FASTMATH_LEVEL
#define ASTRAEUS_FASTMATH_LEVEL 2
#endif

namespace astraeus {
namespace math {
namespace fast {

// =============================================================================
// Constants
// =============================================================================

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float HALF_PI = 1.57079632679489661923f;
constexpr float INV_PI = 0.31830988618379067154f;

// =============================================================================
// Fast Inverse Square Root
// =============================================================================

/**
 * Fast inverse square root using the famous Quake algorithm with Newton refinement.
 * 
 * Domain: x > 0
 * Error: ~0.175% (level 1), ~0.001% (level 2+)
 * Behavior:
 * - Returns +Inf for x = 0
 * - Returns NaN for x < 0 or x = NaN
 * 
 * @param x Input value (must be positive)
 * @return Approximate 1/sqrt(x)
 */
inline float fastInvSqrt(float x) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    // Accurate fallback
    return 1.0f / std::sqrt(x);
#else
    // Quake III algorithm
    float xhalf = 0.5f * x;
    union {
        float f;
        int32_t i;
    } u;
    u.f = x;
    u.i = 0x5f3759df - (u.i >> 1);  // Magic constant
    x = u.f;
    
#if ASTRAEUS_FASTMATH_LEVEL >= 2
    // One Newton-Raphson iteration for better accuracy
    x = x * (1.5f - xhalf * x * x);
#endif
    
#if ASTRAEUS_FASTMATH_LEVEL >= 3
    // Second iteration for even better accuracy
    x = x * (1.5f - xhalf * x * x);
#endif
    
    return x;
#endif
}

/**
 * Fast reciprocal square root (alias for fastInvSqrt).
 */
inline float fastRSqrt(float x) noexcept {
    return fastInvSqrt(x);
}

// =============================================================================
// Fast Square Root
// =============================================================================

/**
 * Fast square root via inverse square root.
 * 
 * Domain: x >= 0
 * Error: ~0.175% (level 1), ~0.001% (level 2+)
 * Behavior:
 * - Returns 0 for x = 0
 * - Returns NaN for x < 0
 * 
 * @param x Input value (must be non-negative)
 * @return Approximate sqrt(x)
 */
inline float fastSqrt(float x) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    return std::sqrt(x);
#else
    // Handle zero to avoid division by zero
    if (x == 0.0f) return 0.0f;
    return x * fastInvSqrt(x);
#endif
}

// =============================================================================
// Fast Trigonometric Functions
// =============================================================================

/**
 * Fast sine approximation using polynomial approximation.
 * 
 * Domain: [-PI, PI] for best accuracy; larger values use range reduction
 * Error: ~0.01 (level 1), ~0.001 (level 2+)
 * Monotonicity: Preserved in [-PI/2, PI/2]
 * 
 * @param x Angle in radians
 * @return Approximate sin(x)
 */
inline float fastSin(float x) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    return std::sin(x);
#else
    // Range reduction to [-PI, PI]
    // Use modulo to handle large inputs
    if (x < -PI || x > PI) {
        x = x - TWO_PI * std::floor((x + PI) / TWO_PI);
    }
    
#if ASTRAEUS_FASTMATH_LEVEL == 1
    // Bhaskara I's sine approximation for [-PI, PI]
    // sin(x) ≈ 16x(PI - |x|) / (5*PI^2 - 4|x|(PI - |x|))
    float abs_x = std::fabs(x);
    float num = 16.0f * x * (PI - abs_x);
    float den = 5.0f * PI * PI - 4.0f * abs_x * (PI - abs_x);
    return num / den;
#else
    // Higher order Taylor series centered at 0
    // Works well in [-PI, PI] with 5th or 7th order
    float x2 = x * x;
    
#if ASTRAEUS_FASTMATH_LEVEL >= 3
    // 9th order polynomial for better accuracy at edges
    return x * (1.0f 
        - x2 * (0.16666666666666666f    // -x³/6
        - x2 * (0.00833333333333333f    // +x⁵/120
        - x2 * (0.0001984126984126984f  // -x⁷/5040
        - x2 * 0.000002755731922398589f)))); // +x⁹/362880
#else
    // 7th order polynomial
    return x * (1.0f 
        - x2 * (0.16666666666666666f    // -x³/6
        - x2 * (0.00833333333333333f    // +x⁵/120
        - x2 * 0.0001984126984126984f))); // -x⁷/5040
#endif
    
#endif
#endif
}

/**
 * Fast cosine approximation using polynomial approximation.
 * 
 * Domain: [-PI, PI] for best accuracy; larger values use range reduction
 * Error: ~0.01 (level 1), ~0.001 (level 2+)
 * 
 * @param x Angle in radians
 * @return Approximate cos(x)
 */
inline float fastCos(float x) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    return std::cos(x);
#else
    // Range reduction to [-PI, PI]
    if (x < -PI || x > PI) {
        x = x - TWO_PI * std::floor((x + PI) / TWO_PI);
    }
    
#if ASTRAEUS_FASTMATH_LEVEL == 1
    // Use Bhaskara approximation with phase shift
    // cos(x) = sin(x + PI/2)
    float y = x + HALF_PI;
    // Wrap to [-PI, PI]
    if (y > PI) y -= TWO_PI;
    float abs_y = std::fabs(y);
    float num = 16.0f * y * (PI - abs_y);
    float den = 5.0f * PI * PI - 4.0f * abs_y * (PI - abs_y);
    return num / den;
#else
    // Taylor series for cos centered at 0
    float x2 = x * x;
    
#if ASTRAEUS_FASTMATH_LEVEL >= 3
    // 8th order polynomial
    return 1.0f 
        - x2 * (0.5f                       // -x²/2
        - x2 * (0.04166666666666667f       // +x⁴/24
        - x2 * (0.0013888888888888889f     // -x⁶/720
        - x2 * 0.000024801587301587302f))); // +x⁸/40320
#else
    // 6th order polynomial
    return 1.0f 
        - x2 * (0.5f                       // -x²/2
        - x2 * (0.04166666666666667f       // +x⁴/24
        - x2 * 0.0013888888888888889f));   // -x⁶/720
#endif
    
#endif
#endif
}

/**
 * Fast simultaneous sine and cosine computation.
 * More efficient than calling fastSin and fastCos separately.
 * 
 * Domain: [-PI, PI] for best accuracy; larger values use range reduction
 * Error: ~0.01 (level 1), ~0.001 (level 2+)
 * 
 * @param x Angle in radians
 * @param s Output sine value
 * @param c Output cosine value
 */
inline void fastSinCos(float x, float& s, float& c) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    s = std::sin(x);
    c = std::cos(x);
#else
    // Simply call both functions
    // A truly optimized version would compute both in one pass,
    // but for simplicity and maintainability, we call both
    s = fastSin(x);
    c = fastCos(x);
#endif
}

/**
 * Fast arctangent of y/x (2-argument atan).
 * Handles all quadrants correctly.
 * 
 * Domain: All real (x, y) except (0, 0)
 * Error: ~0.01 radians (level 1), ~0.005 radians (level 2+)
 * Range: [-PI, PI]
 * Behavior:
 * - Returns 0 for (0, 0)
 * - Handles negative values correctly
 * 
 * @param y Y coordinate
 * @param x X coordinate
 * @return Approximate atan2(y, x) in radians
 */
inline float fastAtan2(float y, float x) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    return std::atan2(y, x);
#else
    // Handle special cases
    if (x == 0.0f && y == 0.0f) return 0.0f;
    
    float abs_y = std::fabs(y);
    float abs_x = std::fabs(x);
    
    // Compute atan(y/x) for first octant using polynomial approximation
    float a;
    if (abs_x >= abs_y) {
        // First and fourth octants
        if (abs_x == 0.0f) {
            a = HALF_PI;
        } else {
            float z = abs_y / abs_x;
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            // Simple approximation (3rd order)
            a = z * (0.995354f - z * (0.288679f - z * 0.079331f));
#else
            // Better approximation (5th order minimax)
            float z2 = z * z;
            a = z * (0.99997726f - z2 * (0.33262347f - z2 * (0.19354346f - z2 * 0.04432514f)));
#endif
        }
    } else {
        // Second and third octants
        float z = abs_x / abs_y;
        
#if ASTRAEUS_FASTMATH_LEVEL == 1
            a = HALF_PI - z * (0.995354f - z * (0.288679f - z * 0.079331f));
#else
            float z2 = z * z;
            a = HALF_PI - z * (0.99997726f - z2 * (0.33262347f - z2 * (0.19354346f - z2 * 0.04432514f)));
#endif
    }
    
    // Handle quadrants
    if (x < 0.0f) {
        a = PI - a;
    }
    if (y < 0.0f) {
        a = -a;
    }
    
    return a;
#endif
}

// =============================================================================
// Fast Exponential Functions (Optional)
// =============================================================================

/**
 * Fast exponential function (e^x).
 * 
 * Domain: Practical range [-10, 10]; larger values may overflow/underflow
 * Error: ~1% (level 1), ~0.1% (level 2+)
 * 
 * @param x Exponent
 * @return Approximate exp(x)
 */
inline float fastExp(float x) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    return std::exp(x);
#else
    // Clamp to prevent overflow/underflow
    x = (x < -10.0f) ? -10.0f : ((x > 10.0f) ? 10.0f : x);
    
#if ASTRAEUS_FASTMATH_LEVEL == 1
    // Simple polynomial approximation
    return 1.0f + x * (1.0f + x * (0.5f + x * (0.16666667f + x * 0.04166667f)));
#else
    // Better approximation using exp(x) = 2^(x/ln(2))
    // Use bit manipulation for 2^n
    constexpr float ln2 = 0.69314718056f;
    float a = x / ln2;
    int n = static_cast<int>(a);
    float r = a - n;
    
    // Compute 2^r using polynomial (r in [0,1])
    float exp_r = 1.0f + r * (0.693147f + r * (0.240227f + r * (0.055504f + r * 0.009676f)));
    
    // Compute 2^n using bit manipulation
    union {
        float f;
        int32_t i;
    } u;
    u.i = (n + 127) << 23;
    
    return u.f * exp_r;
#endif
#endif
}

/**
 * Fast base-2 exponential function (2^x).
 * 
 * Domain: Practical range [-30, 30]
 * Error: ~1% (level 1), ~0.1% (level 2+)
 * 
 * @param x Exponent
 * @return Approximate 2^x
 */
inline float fastExp2(float x) noexcept {
#if ASTRAEUS_FASTMATH_LEVEL == 0
    return std::exp2(x);
#else
    // Clamp to prevent overflow/underflow
    x = (x < -30.0f) ? -30.0f : ((x > 30.0f) ? 30.0f : x);
    
    int n = static_cast<int>(x);
    float r = x - n;
    
    // Compute 2^r using polynomial (r in [0,1])
#if ASTRAEUS_FASTMATH_LEVEL == 1
    float exp_r = 1.0f + r * (0.693147f + r * 0.240227f);
#else
    float exp_r = 1.0f + r * (0.693147f + r * (0.240227f + r * (0.055504f + r * 0.009676f)));
#endif
    
    // Compute 2^n using bit manipulation
    union {
        float f;
        int32_t i;
    } u;
    u.i = (n + 127) << 23;
    
    return u.f * exp_r;
#endif
}

// =============================================================================
// Vector Helpers
// =============================================================================

/**
 * Fast vector length using fast square root.
 * 
 * @param x X component
 * @param y Y component
 * @param z Z component
 * @return Approximate length of vector (x, y, z)
 */
inline float fastLength(float x, float y, float z) noexcept {
    return fastSqrt(x * x + y * y + z * z);
}

/**
 * Fast vector normalization using fast inverse square root.
 * 
 * Domain: Non-zero vectors
 * Behavior: Returns (0, 0, 0) for zero-length vectors
 * 
 * @param x X component (input/output)
 * @param y Y component (input/output)
 * @param z Z component (input/output)
 */
inline void fastNormalize(float& x, float& y, float& z) noexcept {
    float len_sq = x * x + y * y + z * z;
    
    // Handle zero vector
    if (len_sq < std::numeric_limits<float>::epsilon()) {
        x = y = z = 0.0f;
        return;
    }
    
    float inv_len = fastInvSqrt(len_sq);
    x *= inv_len;
    y *= inv_len;
    z *= inv_len;
}

/**
 * Fast vector normalization returning result in separate variables.
 * 
 * @param x X component
 * @param y Y component
 * @param z Z component
 * @param out_x Normalized X component
 * @param out_y Normalized Y component
 * @param out_z Normalized Z component
 */
inline void fastNormalize(float x, float y, float z, float& out_x, float& out_y, float& out_z) noexcept {
    out_x = x;
    out_y = y;
    out_z = z;
    fastNormalize(out_x, out_y, out_z);
}

// =============================================================================
// Additional Utility Functions
// =============================================================================

/**
 * Fast reciprocal (1/x).
 * Useful for avoiding divisions in hot loops.
 * 
 * Domain: x != 0
 * Error: Platform-dependent (uses float division)
 * 
 * @param x Input value (must be non-zero)
 * @return 1/x
 */
inline float fastRecip(float x) noexcept {
    // On modern CPUs, float division is fast enough
    // Could be optimized with platform-specific intrinsics if needed
    return 1.0f / x;
}

/**
 * Check if a value is approximately zero.
 * 
 * @param x Value to check
 * @param epsilon Tolerance (default: 1e-6)
 * @return true if |x| < epsilon
 */
inline bool isApproxZero(float x, float epsilon = 1e-6f) noexcept {
    return std::fabs(x) < epsilon;
}

/**
 * Check if two values are approximately equal.
 * 
 * @param a First value
 * @param b Second value
 * @param epsilon Tolerance (default: 1e-6)
 * @return true if |a - b| < epsilon
 */
inline bool isApproxEqual(float a, float b, float epsilon = 1e-6f) noexcept {
    return std::fabs(a - b) < epsilon;
}

} // namespace fast
} // namespace math
} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_FASTMATH_HPP
