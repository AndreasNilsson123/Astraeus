#include <gtest/gtest.h>
#include "core/Telemetry.hpp"
#include <thread>
#include <chrono>

namespace astraeus {
namespace testing {

class TelemetryTest : public ::testing::Test {
protected:
    void SetUp() override {
        telemetry_ = std::make_unique<Telemetry>();
    }
    
    void TearDown() override {
        telemetry_.reset();
    }
    
    std::unique_ptr<Telemetry> telemetry_;
};

/**
 * Test basic counter increment operations.
 */
TEST_F(TelemetryTest, IncrementCounter) {
    telemetry_->increment_counter("test_counter");
    EXPECT_EQ(telemetry_->get_counter("test_counter"), 1);
    
    telemetry_->increment_counter("test_counter");
    EXPECT_EQ(telemetry_->get_counter("test_counter"), 2);
    
    telemetry_->increment_counter("test_counter", 5);
    EXPECT_EQ(telemetry_->get_counter("test_counter"), 7);
}

/**
 * Test counter with default value for non-existent counter.
 */
TEST_F(TelemetryTest, NonExistentCounter) {
    EXPECT_EQ(telemetry_->get_counter("nonexistent"), 0);
}

/**
 * Test multiple independent counters.
 */
TEST_F(TelemetryTest, MultipleCounters) {
    telemetry_->increment_counter("counter_a");
    telemetry_->increment_counter("counter_b");
    telemetry_->increment_counter("counter_a");
    
    EXPECT_EQ(telemetry_->get_counter("counter_a"), 2);
    EXPECT_EQ(telemetry_->get_counter("counter_b"), 1);
}

/**
 * Test counter reset functionality.
 */
TEST_F(TelemetryTest, ResetCounter) {
    telemetry_->increment_counter("test_counter", 10);
    EXPECT_EQ(telemetry_->get_counter("test_counter"), 10);
    
    telemetry_->reset_counter("test_counter");
    EXPECT_EQ(telemetry_->get_counter("test_counter"), 0);
}

/**
 * Test timing scope basic functionality.
 */
TEST_F(TelemetryTest, TimingScope) {
    {
        auto scope = telemetry_->start_timer("test_operation");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    double elapsed = telemetry_->get_timer_ms("test_operation");
    EXPECT_GE(elapsed, 9.0); // At least 9ms (accounting for timing variance)
    EXPECT_LT(elapsed, 50.0); // But not too long
}

/**
 * Test multiple timing measurements.
 */
TEST_F(TelemetryTest, MultipleTimings) {
    for (int i = 0; i < 5; ++i) {
        auto scope = telemetry_->start_timer("repeated_operation");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Should have recorded multiple samples
    double avg = telemetry_->get_average_timer_ms("repeated_operation");
    EXPECT_GT(avg, 0.0);
}

/**
 * Test frame statistics tracking.
 */
TEST_F(TelemetryTest, FrameStats) {
    telemetry_->begin_frame();
    
    // Simulate some work
    telemetry_->increment_counter("draws");
    telemetry_->increment_counter("draws");
    telemetry_->increment_counter("draws");
    
    telemetry_->end_frame();
    
    EXPECT_EQ(telemetry_->get_frame_count(), 1);
    EXPECT_EQ(telemetry_->get_counter("draws"), 3);
}

/**
 * Test FPS calculation over multiple frames.
 */
TEST_F(TelemetryTest, FPSCalculation) {
    for (int i = 0; i < 10; ++i) {
        telemetry_->begin_frame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        telemetry_->end_frame();
    }
    
    double fps = telemetry_->get_fps();
    EXPECT_GT(fps, 0.0);
    EXPECT_LT(fps, 100.0); // Should be reasonable
}

/**
 * Test reset all telemetry data.
 */
TEST_F(TelemetryTest, ResetAll) {
    telemetry_->increment_counter("counter_a", 10);
    telemetry_->increment_counter("counter_b", 20);
    
    telemetry_->reset_all();
    
    EXPECT_EQ(telemetry_->get_counter("counter_a"), 0);
    EXPECT_EQ(telemetry_->get_counter("counter_b"), 0);
}

/**
 * Test thread safety of counter increments.
 * This test verifies that concurrent counter updates are safe.
 */
TEST_F(TelemetryTest, ThreadSafety) {
    const int num_threads = 4;
    const int increments_per_thread = 1000;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, increments_per_thread]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                telemetry_->increment_counter("thread_safe_counter");
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(telemetry_->get_counter("thread_safe_counter"), 
              num_threads * increments_per_thread);
}

/**
 * Test that timer doesn't crash with zero elapsed time.
 */
TEST_F(TelemetryTest, ZeroElapsedTimer) {
    {
        auto scope = telemetry_->start_timer("instant_operation");
        // No delay
    }
    
    double elapsed = telemetry_->get_timer_ms("instant_operation");
    EXPECT_GE(elapsed, 0.0);
}

} // namespace testing
} // namespace astraeus
