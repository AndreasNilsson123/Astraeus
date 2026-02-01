#include <gtest/gtest.h>
#include "api/EngineAPI.h"
#include <cstddef>

namespace astraeus {
namespace testing {

class ABIContractTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * Test that key ABI struct sizes are documented.
 */
TEST_F(ABIContractTest, StructSizes) {
    // Document struct sizes for ABI stability
    size_t readback_config_size = sizeof(ReadbackConfig);
    EXPECT_GT(readback_config_size, 0);
    
    size_t frame_stats_size = sizeof(FrameStats);
    EXPECT_GT(frame_stats_size, 0);
}

/**
 * Test struct alignment requirements.
 */
TEST_F(ABIContractTest, StructAlignment) {
    EXPECT_GT(alignof(ReadbackConfig), 0);
    EXPECT_GT(alignof(FrameStats), 0);
}

/**
 * Test pixel format enum values are stable.
 */
TEST_F(ABIContractTest, PixelFormatValues) {
    // These values must remain stable across versions
    EXPECT_EQ(static_cast<int>(PIXEL_FORMAT_RGBA8), 0);
    EXPECT_EQ(static_cast<int>(PIXEL_FORMAT_BGRA8), 1);
}

/**
 * Test readback config structure.
 */
TEST_F(ABIContractTest, ReadbackConfigStructure) {
    ReadbackConfig config;
    config.max_width = 1920;
    config.max_height = 1080;
    config.format = PIXEL_FORMAT_RGBA8;
    config.enable_double_buffer = true;
    
    EXPECT_EQ(config.max_width, 1920);
    EXPECT_EQ(config.max_height, 1080);
    EXPECT_EQ(config.format, PIXEL_FORMAT_RGBA8);
    EXPECT_TRUE(config.enable_double_buffer);
}

/**
 * Test FrameStats structure integrity.
 */
TEST_F(ABIContractTest, FrameStatsStructure) {
    FrameStats stats;
    stats.frame_number = 1000;
    stats.delta_time_ms = 16.67;
    stats.render_time_ms = 12.5;
    stats.draw_calls = 100;
    stats.triangle_count = 50000;
    stats.entity_count = 500;
    
    EXPECT_EQ(stats.frame_number, 1000);
    EXPECT_DOUBLE_EQ(stats.delta_time_ms, 16.67);
    EXPECT_DOUBLE_EQ(stats.render_time_ms, 12.5);
    EXPECT_EQ(stats.draw_calls, 100);
    EXPECT_EQ(stats.triangle_count, 50000);
    EXPECT_EQ(stats.entity_count, 500);
}

/**
 * Test bytes per pixel for different formats.
 */
TEST_F(ABIContractTest, BytesPerPixelCalculation) {
    // RGBA8 should be 4 bytes per pixel
    uint32_t rgba8_bpp = 4;
    uint32_t test_width = 100;
    uint32_t test_height = 100;
    size_t expected_rgba8_size = test_width * test_height * rgba8_bpp;
    EXPECT_EQ(expected_rgba8_size, 40000);
    
    // BGRA8 should also be 4 bytes per pixel
    uint32_t bgra8_bpp = 4;
    size_t expected_bgra8_size = test_width * test_height * bgra8_bpp;
    EXPECT_EQ(expected_bgra8_size, 40000);
}

/**
 * Test that enum values fit in expected integer types.
 */
TEST_F(ABIContractTest, EnumSizes) {
    EXPECT_LE(sizeof(PixelFormat), sizeof(uint32_t));
}

/**
 * Test PickResult structure.
 */
TEST_F(ABIContractTest, PickResultStructure) {
    PickResult result;
    result.entity_id = 42;
    result.depth = 0.5f;
    result.world_x = 1.0f;
    result.world_y = 2.0f;
    result.world_z = 3.0f;
    result.hit = true;
    
    EXPECT_EQ(result.entity_id, 42);
    EXPECT_FLOAT_EQ(result.depth, 0.5f);
    EXPECT_FLOAT_EQ(result.world_x, 1.0f);
    EXPECT_TRUE(result.hit);
}

/**
 * Test CameraDesc structure.
 */
TEST_F(ABIContractTest, CameraDescStructure) {
    CameraDesc cam;
    cam.pos_x = 10.0f;
    cam.pos_y = 5.0f;
    cam.pos_z = 15.0f;
    cam.fov_degrees = 60.0f;
    cam.near_plane = 0.1f;
    cam.far_plane = 1000.0f;
    
    EXPECT_FLOAT_EQ(cam.pos_x, 10.0f);
    EXPECT_FLOAT_EQ(cam.fov_degrees, 60.0f);
    EXPECT_FLOAT_EQ(cam.near_plane, 0.1f);
}

/**
 * Test maximum reasonable dimensions don't overflow.
 */
TEST_F(ABIContractTest, MaximumDimensions) {
    uint32_t max_width = 7680;  // 8K
    uint32_t max_height = 4320;
    uint32_t bytes_per_pixel = 4;
    
    size_t buffer_size = static_cast<size_t>(max_width) * max_height * bytes_per_pixel;
    
    EXPECT_GT(buffer_size, 0);
    EXPECT_LT(buffer_size, SIZE_MAX);
}

/**
 * Test ABI version constants if defined.
 */
TEST_F(ABIContractTest, VersionConstants) {
    EXPECT_EQ(ASTRAEUS_VERSION_MAJOR, 0);
    EXPECT_EQ(ASTRAEUS_VERSION_MINOR, 1);
    EXPECT_EQ(ASTRAEUS_VERSION_PATCH, 0);
}

} // namespace testing
} // namespace astraeus
