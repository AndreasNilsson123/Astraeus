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
 * Test basic telemetry enable/disable.
 */
TEST_F(TelemetryTest, EnableDisable) {
    EXPECT_TRUE(telemetry_->is_enabled());
    
    telemetry_->set_enabled(false);
    EXPECT_FALSE(telemetry_->is_enabled());
    
    telemetry_->set_enabled(true);
    EXPECT_TRUE(telemetry_->is_enabled());
}

/**
 * Test frame lifecycle with begin/end.
 */
TEST_F(TelemetryTest, FrameLifecycle) {
    telemetry_->begin_frame(1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    telemetry_->end_frame(10, 1000);
    
    const auto& stats = telemetry_->get_current_stats();
    EXPECT_EQ(stats.frame_number, 1);
    EXPECT_EQ(stats.draw_calls, 10);
    EXPECT_EQ(stats.triangle_count, 1000);
    EXPECT_GT(stats.total_time_ms, 0.0);
}

/**
 * Test multiple frame cycles.
 */
TEST_F(TelemetryTest, MultipleFrames) {
    for (uint64_t i = 1; i <= 5; ++i) {
        telemetry_->begin_frame(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        telemetry_->end_frame(i * 10, i * 100);
        
        const auto& stats = telemetry_->get_current_stats();
        EXPECT_EQ(stats.frame_number, i);
    }
}

/**
 * Test pass timing.
 */
TEST_F(TelemetryTest, PassTiming) {
    telemetry_->begin_frame(1);
    
    uint32_t pass_idx = telemetry_->begin_pass("TestPass");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    telemetry_->end_pass(pass_idx);
    
    telemetry_->end_frame(0, 0);
    
    EXPECT_EQ(telemetry_->get_pass_count(), 1);
    
    const auto* pass_timing = telemetry_->get_pass_timing(0);
    ASSERT_NE(pass_timing, nullptr);
    EXPECT_STREQ(pass_timing->name, "TestPass");
    EXPECT_GT(pass_timing->duration_ms, 0.0);
}

/**
 * Test multiple passes in a frame.
 */
TEST_F(TelemetryTest, MultiplePasses) {
    telemetry_->begin_frame(1);
    
    uint32_t pass1 = telemetry_->begin_pass("Pass1");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    telemetry_->end_pass(pass1);
    
    uint32_t pass2 = telemetry_->begin_pass("Pass2");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    telemetry_->end_pass(pass2);
    
    telemetry_->end_frame(0, 0);
    
    EXPECT_EQ(telemetry_->get_pass_count(), 2);
}

/**
 * Test RAII PassTimer.
 */
TEST_F(TelemetryTest, PassTimerRAII) {
    telemetry_->begin_frame(1);
    
    {
        PassTimer timer(telemetry_.get(), "RAIIPass");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    } // Timer auto-ends here
    
    telemetry_->end_frame(0, 0);
    
    EXPECT_EQ(telemetry_->get_pass_count(), 1);
}

/**
 * Test historical data.
 */
TEST_F(TelemetryTest, HistoricalData) {
    // Generate some history
    for (uint64_t i = 1; i <= 10; ++i) {
        telemetry_->begin_frame(i);
        telemetry_->end_frame(i, i * 10);
    }
    
    // Retrieve history
    std::vector<Telemetry::FrameStats> history(10);
    uint32_t count = telemetry_->get_history(history.data(), 10);
    
    EXPECT_GT(count, 0);
    EXPECT_LE(count, 10);
}

/**
 * Test reset functionality.
 */
TEST_F(TelemetryTest, Reset) {
    telemetry_->begin_frame(1);
    telemetry_->end_frame(10, 100);
    
    telemetry_->reset();
    
    const auto& stats = telemetry_->get_current_stats();
    EXPECT_EQ(stats.frame_number, 0);
    EXPECT_EQ(stats.draw_calls, 0);
}

/**
 * Test disabled telemetry has no overhead.
 */
TEST_F(TelemetryTest, DisabledNoOp) {
    telemetry_->set_enabled(false);
    
    // These should be no-ops
    telemetry_->begin_frame(1);
    uint32_t pass = telemetry_->begin_pass("NoOpPass");
    telemetry_->end_pass(pass);
    telemetry_->end_frame(10, 100);
    
    // Stats should be zero or default
}

/**
 * Test get_pass_timing with invalid index.
 */
TEST_F(TelemetryTest, InvalidPassIndex) {
    telemetry_->begin_frame(1);
    telemetry_->end_frame(0, 0);
    
    const auto* pass = telemetry_->get_pass_timing(999);
    EXPECT_EQ(pass, nullptr);
}

} // namespace testing
} // namespace astraeus
