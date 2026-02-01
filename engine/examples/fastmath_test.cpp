#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <iomanip>
#include <cstdlib>
#include "core/util/FastMath.hpp"

using namespace astraeus::math::fast;

// Test result tracking
struct TestResults {
    int total = 0;
    int passed = 0;
    
    void record(bool pass) {
        total++;
        if (pass) passed++;
    }
    
    void print_summary() const {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Test Summary: " << passed << "/" << total << " tests passed";
        if (passed == total) {
            std::cout << " ✓" << std::endl;
        } else {
            std::cout << " ✗" << std::endl;
        }
        std::cout << "========================================" << std::endl;
    }
};

TestResults g_results;

void print_test_result(const char* test_name, bool passed) {
    if (passed) {
        std::cout << "  ✓ " << test_name << " PASSED" << std::endl;
    } else {
        std::cout << "  ✗ " << test_name << " FAILED" << std::endl;
    }
    g_results.record(passed);
}

// Helper to compute relative error
float relative_error(float approx, float exact) {
    if (exact == 0.0f) {
        return std::fabs(approx);
    }
    return std::fabs((approx - exact) / exact);
}

// Helper to compute absolute error
float absolute_error(float approx, float exact) {
    return std::fabs(approx - exact);
}

// =============================================================================
// Fast Inverse Square Root Tests
// =============================================================================

void test_fast_inv_sqrt() {
    std::cout << "\n=== Fast Inverse Square Root Tests ===" << std::endl;
    
    // Test specific values
    {
        float test_values[] = {1.0f, 4.0f, 9.0f, 16.0f, 0.25f, 100.0f, 0.01f};
        bool all_passed = true;
        
        for (float x : test_values) {
            float approx = fastInvSqrt(x);
            float exact = 1.0f / std::sqrt(x);
            float rel_err = relative_error(approx, exact);
            
            // Error tolerance based on quality level
#if ASTRAEUS_FASTMATH_LEVEL == 0
            const float max_error = 0.00001f;  // 0.001% for accurate fallback
#elif ASTRAEUS_FASTMATH_LEVEL == 1
            const float max_error = 0.002f;  // 0.2%
#elif ASTRAEUS_FASTMATH_LEVEL == 2
            const float max_error = 0.002f;  // 0.2% (Quake + 1 Newton iteration)
#else
            const float max_error = 0.0001f;  // 0.01% (Quake + 2 Newton iterations)
#endif
            
            if (rel_err > max_error) {
                all_passed = false;
                std::cout << "    Failed for x=" << x << ": approx=" << approx 
                          << ", exact=" << exact << ", rel_err=" << (rel_err * 100.0f) << "%" << std::endl;
            }
        }
        
        print_test_result("Specific values", all_passed);
    }
    
    // Test edge cases
    {
#if ASTRAEUS_FASTMATH_LEVEL == 0
        // Only accurate fallback handles edge cases like std::
        bool pass_zero = std::isinf(fastInvSqrt(0.0f));
        print_test_result("Zero handling (returns Inf)", pass_zero);
        
        bool pass_neg = std::isnan(fastInvSqrt(-1.0f));
        print_test_result("Negative handling (returns NaN)", pass_neg);
#else
        // Fast path doesn't guarantee std:: edge case behavior
        // Just verify it doesn't crash
        volatile float result_zero = fastInvSqrt(0.0f);
        volatile float result_neg = fastInvSqrt(-1.0f);
        (void)result_zero;
        (void)result_neg;
        print_test_result("Edge cases (no crash)", true);
#endif
    }
    
    // Test random values
    {
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(0.001f, 1000.0f);
        
        bool all_passed = true;
        float max_rel_err = 0.0f;
        
        for (int i = 0; i < 1000; ++i) {
            float x = dist(rng);
            float approx = fastInvSqrt(x);
            float exact = 1.0f / std::sqrt(x);
            float rel_err = relative_error(approx, exact);
            
            max_rel_err = std::max(max_rel_err, rel_err);
            
#if ASTRAEUS_FASTMATH_LEVEL == 0
            if (rel_err > 0.00001f) all_passed = false;
#elif ASTRAEUS_FASTMATH_LEVEL == 1
            if (rel_err > 0.002f) all_passed = false;
#elif ASTRAEUS_FASTMATH_LEVEL == 2
            if (rel_err > 0.002f) all_passed = false;
#else
            if (rel_err > 0.0001f) all_passed = false;
#endif
        }
        
        std::cout << "    Max relative error: " << (max_rel_err * 100.0f) << "%" << std::endl;
        print_test_result("Random values (1000 samples)", all_passed);
    }
}

// =============================================================================
// Fast Square Root Tests
// =============================================================================

void test_fast_sqrt() {
    std::cout << "\n=== Fast Square Root Tests ===" << std::endl;
    
    // Test specific values
    {
        float test_values[] = {0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 100.0f, 0.25f, 0.01f};
        bool all_passed = true;
        
        for (float x : test_values) {
            float approx = fastSqrt(x);
            float exact = std::sqrt(x);
            float rel_err = (exact != 0.0f) ? relative_error(approx, exact) : absolute_error(approx, exact);
            
#if ASTRAEUS_FASTMATH_LEVEL == 0
            const float max_error = 0.00001f;
#elif ASTRAEUS_FASTMATH_LEVEL == 1
            const float max_error = 0.002f;
#elif ASTRAEUS_FASTMATH_LEVEL == 2
            const float max_error = 0.002f;
#else
            const float max_error = 0.0001f;
#endif
            
            if (rel_err > max_error) {
                all_passed = false;
                std::cout << "    Failed for x=" << x << ": approx=" << approx 
                          << ", exact=" << exact << ", rel_err=" << (rel_err * 100.0f) << "%" << std::endl;
            }
        }
        
        print_test_result("Specific values", all_passed);
    }
    
    // Test zero exactly
    {
        bool pass = fastSqrt(0.0f) == 0.0f;
        print_test_result("Zero handling (returns 0)", pass);
    }
    
    // Test negative (should return NaN)
    {
#if ASTRAEUS_FASTMATH_LEVEL == 0
        bool pass = std::isnan(fastSqrt(-1.0f));
        print_test_result("Negative handling (returns NaN)", pass);
#else
        // Fast path may not handle this like std::
        volatile float result = fastSqrt(-1.0f);
        (void)result;
        print_test_result("Negative handling (no crash)", true);
#endif
    }
}

// =============================================================================
// Fast Trigonometric Tests
// =============================================================================

void test_fast_sin() {
    std::cout << "\n=== Fast Sine Tests ===" << std::endl;
    
    // Test key angles
    {
        struct TestCase { float x; float expected; };
        TestCase cases[] = {
            {0.0f, 0.0f},
            {PI / 6.0f, 0.5f},
            {PI / 4.0f, 0.707106781f},
            {PI / 3.0f, 0.866025404f},
            {PI / 2.0f, 1.0f},
            {PI, 0.0f},
            {-PI / 2.0f, -1.0f},
        };
        
        bool all_passed = true;
        
        for (const auto& tc : cases) {
            float approx = fastSin(tc.x);
            float exact = std::sin(tc.x);
            float abs_err = absolute_error(approx, exact);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            const float max_error = 0.001f;
#elif ASTRAEUS_FASTMATH_LEVEL == 2
            const float max_error = 0.0001f;
#else
            const float max_error = 0.00005f;
#endif
            
            if (abs_err > max_error) {
                all_passed = false;
                std::cout << "    Failed for x=" << tc.x << ": approx=" << approx 
                          << ", exact=" << exact << ", abs_err=" << abs_err << std::endl;
            }
        }
        
        print_test_result("Key angles", all_passed);
    }
    
    // Test range [-PI, PI]
    {
        bool all_passed = true;
        float max_abs_err = 0.0f;
        
        for (float x = -PI; x <= PI; x += 0.1f) {
            float approx = fastSin(x);
            float exact = std::sin(x);
            float abs_err = absolute_error(approx, exact);
            
            max_abs_err = std::max(max_abs_err, abs_err);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            if (abs_err > 0.01f) all_passed = false;
#elif ASTRAEUS_FASTMATH_LEVEL == 2
            if (abs_err > 0.001f) all_passed = false;
#else
            if (abs_err > 0.0001f) all_passed = false;
#endif
        }
        
        std::cout << "    Max absolute error: " << max_abs_err << std::endl;
        print_test_result("Range [-PI, PI]", all_passed);
    }
    
    // Test large values (range reduction)
    {
        float test_values[] = {10.0f, 100.0f, -50.0f, 1000.0f};
        bool all_passed = true;
        
        for (float x : test_values) {
            float approx = fastSin(x);
            float exact = std::sin(x);
            float abs_err = absolute_error(approx, exact);
            
            // Larger error tolerance for range-reduced values
            if (abs_err > 0.01f) {
                all_passed = false;
                std::cout << "    Failed for x=" << x << ": approx=" << approx 
                          << ", exact=" << exact << ", abs_err=" << abs_err << std::endl;
            }
        }
        
        print_test_result("Large values (range reduction)", all_passed);
    }
}

void test_fast_cos() {
    std::cout << "\n=== Fast Cosine Tests ===" << std::endl;
    
    // Test key angles
    {
        struct TestCase { float x; float expected; };
        TestCase cases[] = {
            {0.0f, 1.0f},
            {PI / 6.0f, 0.866025404f},
            {PI / 4.0f, 0.707106781f},
            {PI / 3.0f, 0.5f},
            {PI / 2.0f, 0.0f},
            {PI, -1.0f},
        };
        
        bool all_passed = true;
        
        for (const auto& tc : cases) {
            float approx = fastCos(tc.x);
            float exact = std::cos(tc.x);
            float abs_err = absolute_error(approx, exact);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            const float max_error = 0.001f;
#elif ASTRAEUS_FASTMATH_LEVEL == 2
            const float max_error = 0.0001f;
#else
            const float max_error = 0.00005f;
#endif
            
            if (abs_err > max_error) {
                all_passed = false;
                std::cout << "    Failed for x=" << tc.x << ": approx=" << approx 
                          << ", exact=" << exact << ", abs_err=" << abs_err << std::endl;
            }
        }
        
        print_test_result("Key angles", all_passed);
    }
    
    // Test range [-PI, PI]
    {
        bool all_passed = true;
        float max_abs_err = 0.0f;
        
        for (float x = -PI; x <= PI; x += 0.1f) {
            float approx = fastCos(x);
            float exact = std::cos(x);
            float abs_err = absolute_error(approx, exact);
            
            max_abs_err = std::max(max_abs_err, abs_err);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            if (abs_err > 0.01f) all_passed = false;
#elif ASTRAEUS_FASTMATH_LEVEL == 2
            if (abs_err > 0.001f) all_passed = false;
#else
            if (abs_err > 0.0001f) all_passed = false;
#endif
        }
        
        std::cout << "    Max absolute error: " << max_abs_err << std::endl;
        print_test_result("Range [-PI, PI]", all_passed);
    }
}

void test_fast_sincos() {
    std::cout << "\n=== Fast SinCos Tests ===" << std::endl;
    
    // Test that fastSinCos matches individual calls
    {
        bool all_passed = true;
        
        for (float x = -PI; x <= PI; x += 0.5f) {
            float s, c;
            fastSinCos(x, s, c);
            
            float s_expected = fastSin(x);
            float c_expected = fastCos(x);
            
            if (absolute_error(s, s_expected) > 1e-6f || absolute_error(c, c_expected) > 1e-6f) {
                all_passed = false;
                std::cout << "    Failed for x=" << x << std::endl;
            }
        }
        
        print_test_result("Consistency with fastSin/fastCos", all_passed);
    }
    
    // Test identity: sin²(x) + cos²(x) = 1
    {
        bool all_passed = true;
        float max_err = 0.0f;
        
        for (float x = -PI; x <= PI; x += 0.1f) {
            float s, c;
            fastSinCos(x, s, c);
            float identity = s * s + c * c;
            float err = std::fabs(identity - 1.0f);
            
            max_err = std::max(max_err, err);
            
            if (err > 0.01f) {
                all_passed = false;
            }
        }
        
        std::cout << "    Max identity error: " << max_err << std::endl;
        print_test_result("Identity sin²+cos²=1", all_passed);
    }
}

void test_fast_atan2() {
    std::cout << "\n=== Fast Atan2 Tests ===" << std::endl;
    
    // Test key angles
    {
        struct TestCase { float y, x; };
        TestCase cases[] = {
            {0.0f, 1.0f},   // 0
            {1.0f, 1.0f},   // PI/4
            {1.0f, 0.0f},   // PI/2
            {1.0f, -1.0f},  // 3*PI/4
            {0.0f, -1.0f},  // PI
            {-1.0f, -1.0f}, // -3*PI/4
            {-1.0f, 0.0f},  // -PI/2
            {-1.0f, 1.0f},  // -PI/4
        };
        
        bool all_passed = true;
        
        for (const auto& tc : cases) {
            float approx = fastAtan2(tc.y, tc.x);
            float exact = std::atan2(tc.y, tc.x);
            float abs_err = absolute_error(approx, exact);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            const float max_error = 0.05f;  // ~2.86 degrees
#else
            const float max_error = 0.01f;  // ~0.57 degrees
#endif
            
            if (abs_err > max_error) {
                all_passed = false;
                std::cout << "    Failed for y=" << tc.y << ", x=" << tc.x 
                          << ": approx=" << approx << ", exact=" << exact 
                          << ", abs_err=" << abs_err << " (" << (abs_err * 180.0f / PI) << " deg)" << std::endl;
            }
        }
        
        print_test_result("Key angles", all_passed);
    }
    
    // Test random values
    {
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
        
        bool all_passed = true;
        float max_abs_err = 0.0f;
        
        for (int i = 0; i < 1000; ++i) {
            float x = dist(rng);
            float y = dist(rng);
            
            // Skip near-zero cases
            if (std::fabs(x) < 0.01f && std::fabs(y) < 0.01f) continue;
            
            float approx = fastAtan2(y, x);
            float exact = std::atan2(y, x);
            float abs_err = absolute_error(approx, exact);
            
            max_abs_err = std::max(max_abs_err, abs_err);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            if (abs_err > 0.05f) all_passed = false;
#else
            if (abs_err > 0.01f) all_passed = false;
#endif
        }
        
        std::cout << "    Max absolute error: " << max_abs_err << " (" 
                  << (max_abs_err * 180.0f / PI) << " deg)" << std::endl;
        print_test_result("Random values (1000 samples)", all_passed);
    }
    
    // Test zero handling
    {
        bool pass = fastAtan2(0.0f, 0.0f) == 0.0f;
        print_test_result("Zero handling (0, 0) returns 0", pass);
    }
}

// =============================================================================
// Fast Exponential Tests
// =============================================================================

void test_fast_exp() {
    std::cout << "\n=== Fast Exponential Tests ===" << std::endl;
    
    // Test specific values
    {
        float test_values[] = {0.0f, 1.0f, 2.0f, -1.0f, -2.0f, 0.5f, -0.5f};
        bool all_passed = true;
        
        for (float x : test_values) {
            float approx = fastExp(x);
            float exact = std::exp(x);
            float rel_err = relative_error(approx, exact);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            const float max_error = 0.015f;  // 1.5%
#else
            const float max_error = 0.002f;  // 0.2%
#endif
            
            if (rel_err > max_error) {
                all_passed = false;
                std::cout << "    Failed for x=" << x << ": approx=" << approx 
                          << ", exact=" << exact << ", rel_err=" << (rel_err * 100.0f) << "%" << std::endl;
            }
        }
        
        print_test_result("Specific values", all_passed);
    }
    
    // Test range [-5, 5]
    {
        bool all_passed = true;
        float max_rel_err = 0.0f;
        
        for (float x = -5.0f; x <= 5.0f; x += 0.5f) {
            float approx = fastExp(x);
            float exact = std::exp(x);
            float rel_err = relative_error(approx, exact);
            
            max_rel_err = std::max(max_rel_err, rel_err);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            if (rel_err > 0.015f) all_passed = false;
#else
            if (rel_err > 0.002f) all_passed = false;
#endif
        }
        
        std::cout << "    Max relative error: " << (max_rel_err * 100.0f) << "%" << std::endl;
        print_test_result("Range [-5, 5]", all_passed);
    }
}

void test_fast_exp2() {
    std::cout << "\n=== Fast Exp2 Tests ===" << std::endl;
    
    // Test specific values
    {
        float test_values[] = {0.0f, 1.0f, 2.0f, 3.0f, -1.0f, -2.0f, 0.5f};
        bool all_passed = true;
        
        for (float x : test_values) {
            float approx = fastExp2(x);
            float exact = std::exp2(x);
            float rel_err = relative_error(approx, exact);
            
#if ASTRAEUS_FASTMATH_LEVEL == 1
            const float max_error = 0.015f;
#else
            const float max_error = 0.002f;
#endif
            
            if (rel_err > max_error) {
                all_passed = false;
                std::cout << "    Failed for x=" << x << ": approx=" << approx 
                          << ", exact=" << exact << ", rel_err=" << (rel_err * 100.0f) << "%" << std::endl;
            }
        }
        
        print_test_result("Specific values", all_passed);
    }
}

// =============================================================================
// Vector Helper Tests
// =============================================================================

void test_fast_length() {
    std::cout << "\n=== Fast Length Tests ===" << std::endl;
    
    // Test specific vectors
    {
        struct TestCase { float x, y, z; };
        TestCase cases[] = {
            {0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {3.0f, 4.0f, 0.0f},  // Should be 5
            {1.0f, 2.0f, 2.0f},  // Should be 3
        };
        
        bool all_passed = true;
        
        for (const auto& tc : cases) {
            float approx = fastLength(tc.x, tc.y, tc.z);
            float exact = std::sqrt(tc.x * tc.x + tc.y * tc.y + tc.z * tc.z);
            float abs_err = absolute_error(approx, exact);
            
            // Use absolute error for small values, relative for large
            float threshold = (exact < 1.0f) ? 0.01f : exact * 0.01f;
            
            if (abs_err > threshold) {
                all_passed = false;
                std::cout << "    Failed for (" << tc.x << ", " << tc.y << ", " << tc.z 
                          << "): approx=" << approx << ", exact=" << exact << std::endl;
            }
        }
        
        print_test_result("Specific vectors", all_passed);
    }
}

void test_fast_normalize() {
    std::cout << "\n=== Fast Normalize Tests ===" << std::endl;
    
    // Test that normalized vectors have unit length
    {
        struct TestCase { float x, y, z; };
        TestCase cases[] = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {3.0f, 4.0f, 0.0f},
            {1.0f, 2.0f, 3.0f},
            {-1.0f, -1.0f, -1.0f},
        };
        
        bool all_passed = true;
        
        for (auto tc : cases) {
            fastNormalize(tc.x, tc.y, tc.z);
            float len = std::sqrt(tc.x * tc.x + tc.y * tc.y + tc.z * tc.z);
            float err = std::fabs(len - 1.0f);
            
            if (err > 0.01f) {
                all_passed = false;
                std::cout << "    Failed: length=" << len << ", err=" << err << std::endl;
            }
        }
        
        print_test_result("Unit length after normalization", all_passed);
    }
    
    // Test zero vector
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        fastNormalize(x, y, z);
        bool pass = (x == 0.0f && y == 0.0f && z == 0.0f);
        print_test_result("Zero vector handling", pass);
    }
}

// =============================================================================
// Utility Function Tests
// =============================================================================

void test_utility_functions() {
    std::cout << "\n=== Utility Function Tests ===" << std::endl;
    
    // Test fastRecip
    {
        bool pass = std::fabs(fastRecip(2.0f) - 0.5f) < 1e-6f;
        print_test_result("fastRecip(2) = 0.5", pass);
    }
    
    // Test isApproxZero
    {
        bool pass1 = isApproxZero(0.0f);
        bool pass2 = isApproxZero(1e-7f);
        bool pass3 = !isApproxZero(0.1f);
        print_test_result("isApproxZero", pass1 && pass2 && pass3);
    }
    
    // Test isApproxEqual
    {
        bool pass1 = isApproxEqual(1.0f, 1.0f);
        bool pass2 = isApproxEqual(1.0f, 1.0f + 1e-7f);
        bool pass3 = !isApproxEqual(1.0f, 1.1f);
        print_test_result("isApproxEqual", pass1 && pass2 && pass3);
    }
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Astraeus Fast Math Unit Tests" << std::endl;
    std::cout << "  Quality Level: " << ASTRAEUS_FASTMATH_LEVEL << std::endl;
    std::cout << "========================================" << std::endl;
    
    test_fast_inv_sqrt();
    test_fast_sqrt();
    test_fast_sin();
    test_fast_cos();
    test_fast_sincos();
    test_fast_atan2();
    test_fast_exp();
    test_fast_exp2();
    test_fast_length();
    test_fast_normalize();
    test_utility_functions();
    
    g_results.print_summary();
    
    return (g_results.passed == g_results.total) ? 0 : 1;
}
