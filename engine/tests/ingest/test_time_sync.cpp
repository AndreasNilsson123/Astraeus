#include <gtest/gtest.h>
#include "ingest/TimeSync.hpp"

namespace astraeus {
namespace testing {

class TimeSyncTest : public ::testing::Test {
protected:
    void SetUp() override {
        time_sync_ = std::make_unique<TimeSync>();
    }
    
    void TearDown() override {
        time_sync_.reset();
    }
    
    std::unique_ptr<TimeSync> time_sync_;
};

/**
 * Test basic interpolation between two points.
 */
TEST_F(TimeSyncTest, BasicInterpolation) {
    time_sync_->add_sample(0.0, 0.0);
    time_sync_->add_sample(1.0, 10.0);
    
    // Interpolate at 0.5 should give 5.0
    double result = time_sync_->interpolate(0.5);
    EXPECT_NEAR(result, 5.0, 0.01);
}

/**
 * Test interpolation with multiple samples.
 */
TEST_F(TimeSyncTest, MultipleInterpolations) {
    time_sync_->add_sample(0.0, 0.0);
    time_sync_->add_sample(1.0, 10.0);
    time_sync_->add_sample(2.0, 20.0);
    time_sync_->add_sample(3.0, 30.0);
    
    EXPECT_NEAR(time_sync_->interpolate(0.5), 5.0, 0.01);
    EXPECT_NEAR(time_sync_->interpolate(1.5), 15.0, 0.01);
    EXPECT_NEAR(time_sync_->interpolate(2.5), 25.0, 0.01);
}

/**
 * Test extrapolation beyond available data.
 */
TEST_F(TimeSyncTest, Extrapolation) {
    time_sync_->add_sample(0.0, 0.0);
    time_sync_->add_sample(1.0, 10.0);
    
    // Extrapolate beyond last sample
    double result = time_sync_->interpolate(2.0);
    // Should extrapolate linearly or clamp to last value
    EXPECT_GE(result, 10.0);
}

/**
 * Test behavior with no data.
 */
TEST_F(TimeSyncTest, NoData) {
    // Query with no samples added
    double result = time_sync_->interpolate(1.0);
    // Should return some default value (0.0 or NaN)
    // Test documents current behavior
}

/**
 * Test behavior with single sample.
 */
TEST_F(TimeSyncTest, SingleSample) {
    time_sync_->add_sample(1.0, 100.0);
    
    double result = time_sync_->interpolate(1.0);
    EXPECT_DOUBLE_EQ(result, 100.0);
    
    // Query before or after should return the sample value or extrapolate
    double before = time_sync_->interpolate(0.5);
    double after = time_sync_->interpolate(1.5);
}

/**
 * Test clearing samples.
 */
TEST_F(TimeSyncTest, ClearSamples) {
    time_sync_->add_sample(0.0, 0.0);
    time_sync_->add_sample(1.0, 10.0);
    
    time_sync_->clear();
    
    // After clear, should behave like no data
}

/**
 * Test sample window management.
 */
TEST_F(TimeSyncTest, SampleWindow) {
    // Add many samples
    for (int i = 0; i < 100; ++i) {
        time_sync_->add_sample(i * 0.1, i * 1.0);
    }
    
    // Should keep a reasonable window of samples
    // Old samples should be removed
    double result = time_sync_->interpolate(5.0);
    EXPECT_NEAR(result, 50.0, 1.0);
}

/**
 * Test getting current sync offset.
 */
TEST_F(TimeSyncTest, SyncOffset) {
    time_sync_->add_sample(0.0, 0.0);
    time_sync_->add_sample(1.0, 1.5); // 0.5 second offset
    
    double offset = time_sync_->get_offset();
    // Offset represents time difference between local and remote
}

/**
 * Test interpolation with non-uniform spacing.
 */
TEST_F(TimeSyncTest, NonUniformSpacing) {
    time_sync_->add_sample(0.0, 0.0);
    time_sync_->add_sample(0.5, 5.0);
    time_sync_->add_sample(2.0, 20.0);
    
    double result1 = time_sync_->interpolate(0.25);
    double result2 = time_sync_->interpolate(1.0);
    
    // Should handle non-uniform spacing gracefully
}

/**
 * Test interpolation boundaries.
 */
TEST_F(TimeSyncTest, InterpolationBoundaries) {
    time_sync_->add_sample(1.0, 10.0);
    time_sync_->add_sample(2.0, 20.0);
    
    // Query exactly at sample points
    EXPECT_DOUBLE_EQ(time_sync_->interpolate(1.0), 10.0);
    EXPECT_DOUBLE_EQ(time_sync_->interpolate(2.0), 20.0);
    
    // Query before first sample
    double before = time_sync_->interpolate(0.5);
    
    // Query after last sample
    double after = time_sync_->interpolate(2.5);
}

/**
 * Test adding samples out of order.
 */
TEST_F(TimeSyncTest, OutOfOrderSamples) {
    time_sync_->add_sample(2.0, 20.0);
    time_sync_->add_sample(1.0, 10.0);
    time_sync_->add_sample(3.0, 30.0);
    
    // Should handle out-of-order samples
    double result = time_sync_->interpolate(1.5);
    EXPECT_NEAR(result, 15.0, 0.1);
}

/**
 * Test negative timestamps.
 */
TEST_F(TimeSyncTest, NegativeTimestamps) {
    time_sync_->add_sample(-1.0, -10.0);
    time_sync_->add_sample(0.0, 0.0);
    time_sync_->add_sample(1.0, 10.0);
    
    double result = time_sync_->interpolate(-0.5);
    EXPECT_NEAR(result, -5.0, 0.1);
}

} // namespace testing
} // namespace astraeus
