#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <random>
#include <iomanip>
#include <sstream>

#include "core/util/FastMath.hpp"

using namespace astraeus::math::fast;
using namespace std::chrono;

// Benchmark result structure
struct BenchmarkResult {
    std::string name;
    double time_ms;
    double speedup;
};

std::vector<BenchmarkResult> g_results;

// Helper to format numbers
std::string format_time(double ms) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << ms << " ms";
    return oss.str();
}

std::string format_speedup(double speedup) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << speedup << "x";
    return oss.str();
}

// Generic benchmark function
template <typename Func>
double benchmark(const char* name, Func func, int iterations = 1000000) {
    // Warm-up
    for (int i = 0; i < 1000; ++i) {
        func(i);
    }
    
    // Actual benchmark
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func(i);
    }
    auto end = high_resolution_clock::now();
    
    double time_ms = duration<double, std::milli>(end - start).count();
    
    std::cout << "  " << std::left << std::setw(40) << name 
              << std::right << std::setw(12) << format_time(time_ms) << std::endl;
    
    return time_ms;
}

// Prevent compiler optimization
volatile float g_sink = 0.0f;

void sink(float value) {
    g_sink = value;
}

// =============================================================================
// Inverse Square Root Benchmarks
// =============================================================================

void benchmark_inv_sqrt() {
    std::cout << "\n=== Inverse Square Root Benchmark ===" << std::endl;
    
    // Generate test data
    std::vector<float> test_data(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.1f, 100.0f);
    for (auto& val : test_data) {
        val = dist(rng);
    }
    
    // Benchmark std::sqrt
    double std_time = benchmark("std::sqrt (1/sqrt(x))", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = 1.0f / std::sqrt(x);
        sink(result);
    });
    
    // Benchmark fastInvSqrt
    double fast_time = benchmark("fastInvSqrt", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = fastInvSqrt(x);
        sink(result);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastInvSqrt", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

// =============================================================================
// Square Root Benchmarks
// =============================================================================

void benchmark_sqrt() {
    std::cout << "\n=== Square Root Benchmark ===" << std::endl;
    
    std::vector<float> test_data(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.1f, 100.0f);
    for (auto& val : test_data) {
        val = dist(rng);
    }
    
    double std_time = benchmark("std::sqrt", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = std::sqrt(x);
        sink(result);
    });
    
    double fast_time = benchmark("fastSqrt", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = fastSqrt(x);
        sink(result);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastSqrt", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

// =============================================================================
// Trigonometric Benchmarks
// =============================================================================

void benchmark_sin() {
    std::cout << "\n=== Sine Benchmark ===" << std::endl;
    
    std::vector<float> test_data(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-PI, PI);
    for (auto& val : test_data) {
        val = dist(rng);
    }
    
    double std_time = benchmark("std::sin", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = std::sin(x);
        sink(result);
    });
    
    double fast_time = benchmark("fastSin", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = fastSin(x);
        sink(result);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastSin", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

void benchmark_cos() {
    std::cout << "\n=== Cosine Benchmark ===" << std::endl;
    
    std::vector<float> test_data(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-PI, PI);
    for (auto& val : test_data) {
        val = dist(rng);
    }
    
    double std_time = benchmark("std::cos", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = std::cos(x);
        sink(result);
    });
    
    double fast_time = benchmark("fastCos", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = fastCos(x);
        sink(result);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastCos", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

void benchmark_sincos() {
    std::cout << "\n=== SinCos Benchmark ===" << std::endl;
    
    std::vector<float> test_data(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-PI, PI);
    for (auto& val : test_data) {
        val = dist(rng);
    }
    
    double std_time = benchmark("std::sin + std::cos", [&](int i) {
        float x = test_data[i % test_data.size()];
        float s = std::sin(x);
        float c = std::cos(x);
        sink(s + c);
    });
    
    double fast_separate_time = benchmark("fastSin + fastCos", [&](int i) {
        float x = test_data[i % test_data.size()];
        float s = fastSin(x);
        float c = fastCos(x);
        sink(s + c);
    });
    
    double fast_combined_time = benchmark("fastSinCos", [&](int i) {
        float x = test_data[i % test_data.size()];
        float s, c;
        fastSinCos(x, s, c);
        sink(s + c);
    });
    
    double speedup = std_time / fast_combined_time;
    g_results.push_back({"fastSinCos", fast_combined_time, speedup});
    
    std::cout << "  Speedup vs std: " << format_speedup(speedup) << std::endl;
    std::cout << "  Speedup vs separate: " << format_speedup(fast_separate_time / fast_combined_time) << std::endl;
}

void benchmark_atan2() {
    std::cout << "\n=== Atan2 Benchmark ===" << std::endl;
    
    std::vector<float> test_data_y(1000);
    std::vector<float> test_data_x(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    for (size_t i = 0; i < test_data_y.size(); ++i) {
        test_data_y[i] = dist(rng);
        test_data_x[i] = dist(rng);
    }
    
    double std_time = benchmark("std::atan2", [&](int i) {
        size_t idx = i % test_data_y.size();
        float result = std::atan2(test_data_y[idx], test_data_x[idx]);
        sink(result);
    });
    
    double fast_time = benchmark("fastAtan2", [&](int i) {
        size_t idx = i % test_data_y.size();
        float result = fastAtan2(test_data_y[idx], test_data_x[idx]);
        sink(result);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastAtan2", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

// =============================================================================
// Exponential Benchmarks
// =============================================================================

void benchmark_exp() {
    std::cout << "\n=== Exponential Benchmark ===" << std::endl;
    
    std::vector<float> test_data(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-5.0f, 5.0f);
    for (auto& val : test_data) {
        val = dist(rng);
    }
    
    double std_time = benchmark("std::exp", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = std::exp(x);
        sink(result);
    });
    
    double fast_time = benchmark("fastExp", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = fastExp(x);
        sink(result);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastExp", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

void benchmark_exp2() {
    std::cout << "\n=== Exp2 Benchmark ===" << std::endl;
    
    std::vector<float> test_data(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-5.0f, 5.0f);
    for (auto& val : test_data) {
        val = dist(rng);
    }
    
    double std_time = benchmark("std::exp2", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = std::exp2(x);
        sink(result);
    });
    
    double fast_time = benchmark("fastExp2", [&](int i) {
        float x = test_data[i % test_data.size()];
        float result = fastExp2(x);
        sink(result);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastExp2", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

// =============================================================================
// Vector Operations Benchmarks
// =============================================================================

void benchmark_normalize() {
    std::cout << "\n=== Vector Normalize Benchmark ===" << std::endl;
    
    std::vector<float> test_x(1000), test_y(1000), test_z(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    for (size_t i = 0; i < test_x.size(); ++i) {
        test_x[i] = dist(rng);
        test_y[i] = dist(rng);
        test_z[i] = dist(rng);
    }
    
    double std_time = benchmark("std::sqrt normalize", [&](int i) {
        size_t idx = i % test_x.size();
        float x = test_x[idx];
        float y = test_y[idx];
        float z = test_z[idx];
        float len = std::sqrt(x * x + y * y + z * z);
        if (len > 0.0f) {
            float inv_len = 1.0f / len;
            x *= inv_len;
            y *= inv_len;
            z *= inv_len;
        }
        sink(x + y + z);
    });
    
    double fast_time = benchmark("fastNormalize", [&](int i) {
        size_t idx = i % test_x.size();
        float x = test_x[idx];
        float y = test_y[idx];
        float z = test_z[idx];
        fastNormalize(x, y, z);
        sink(x + y + z);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastNormalize", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

void benchmark_length() {
    std::cout << "\n=== Vector Length Benchmark ===" << std::endl;
    
    std::vector<float> test_x(1000), test_y(1000), test_z(1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    for (size_t i = 0; i < test_x.size(); ++i) {
        test_x[i] = dist(rng);
        test_y[i] = dist(rng);
        test_z[i] = dist(rng);
    }
    
    double std_time = benchmark("std::sqrt length", [&](int i) {
        size_t idx = i % test_x.size();
        float x = test_x[idx];
        float y = test_y[idx];
        float z = test_z[idx];
        float len = std::sqrt(x * x + y * y + z * z);
        sink(len);
    });
    
    double fast_time = benchmark("fastLength", [&](int i) {
        size_t idx = i % test_x.size();
        float x = test_x[idx];
        float y = test_y[idx];
        float z = test_z[idx];
        float len = fastLength(x, y, z);
        sink(len);
    });
    
    double speedup = std_time / fast_time;
    g_results.push_back({"fastLength", fast_time, speedup});
    
    std::cout << "  Speedup: " << format_speedup(speedup) << std::endl;
}

// =============================================================================
// Summary
// =============================================================================

void print_summary() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Benchmark Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::left << std::setw(20) << "Function" 
              << std::right << std::setw(15) << "Time" 
              << std::right << std::setw(15) << "Speedup" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    for (const auto& result : g_results) {
        std::cout << std::left << std::setw(20) << result.name
                  << std::right << std::setw(15) << format_time(result.time_ms)
                  << std::right << std::setw(15) << format_speedup(result.speedup) << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    
    // Calculate average speedup
    if (!g_results.empty()) {
        double total_speedup = 0.0;
        for (const auto& result : g_results) {
            total_speedup += result.speedup;
        }
        double avg_speedup = total_speedup / g_results.size();
        std::cout << "Average speedup: " << format_speedup(avg_speedup) << std::endl;
    }
}

// =============================================================================
// Main
// =============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Astraeus Fast Math Benchmarks" << std::endl;
    std::cout << "  Quality Level: " << ASTRAEUS_FASTMATH_LEVEL << std::endl;
    std::cout << "========================================" << std::endl;
    
    benchmark_inv_sqrt();
    benchmark_sqrt();
    benchmark_sin();
    benchmark_cos();
    benchmark_sincos();
    benchmark_atan2();
    benchmark_exp();
    benchmark_exp2();
    benchmark_normalize();
    benchmark_length();
    
    print_summary();
    
    return 0;
}
