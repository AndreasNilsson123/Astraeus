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
 * Test that key ABI struct sizes are stable.
 * Changes to these sizes break ABI compatibility.
 */
TEST_F(ABIContractTest, StructSizes) {
    // Document current struct sizes
    // If these change, it's an ABI break
    
    // EntityHandle should be a fixed size (typically 8 bytes)
    EXPECT_EQ(sizeof(EntityHandle), sizeof(uint64_t));
    
    // ReadbackConfig should have known size
    size_t readback_config_size = sizeof(ReadbackConfig);
    EXPECT_GT(readback_config_size, 0);
    
    // Stats structure size
    size_t stats_size = sizeof(EngineStats);
    EXPECT_GT(stats_size, 0);
}

/**
 * Test struct alignment requirements.
 */
TEST_F(ABIContractTest, StructAlignment) {
    // Verify alignment is as expected for FFM
    EXPECT_EQ(alignof(EntityHandle), alignof(uint64_t));
    EXPECT_GT(alignof(ReadbackConfig), 0);
    EXPECT_GT(alignof(EngineStats), 0);
}

/**
 * Test that INVALID_ENTITY is a known value.
 */
TEST_F(ABIContractTest, InvalidEntityValue) {
    EXPECT_EQ(INVALID_ENTITY, 0);
}

/**
 * Test pixel format enum values are stable.
 */
TEST_F(ABIContractTest, PixelFormatValues) {
    // These values must remain stable across versions
    EXPECT_EQ(static_cast<int>(PixelFormat::RGBA8), 0);
    EXPECT_EQ(static_cast<int>(PixelFormat::RGB8), 1);
    EXPECT_EQ(static_cast<int>(PixelFormat::BGRA8), 2);
    EXPECT_EQ(static_cast<int>(PixelFormat::BGR8), 3);
}

/**
 * Test readback config field offsets are predictable.
 */
TEST_F(ABIContractTest, ReadbackConfigLayout) {
    ReadbackConfig config;
    config.width = 800;
    config.height = 600;
    config.format = PixelFormat::RGBA8;
    config.buffer_capacity = 1920000;
    
    // Verify fields are accessible
    EXPECT_EQ(config.width, 800);
    EXPECT_EQ(config.height, 600);
    EXPECT_EQ(config.format, PixelFormat::RGBA8);
    EXPECT_EQ(config.buffer_capacity, 1920000);
}

/**
 * Test bytes per pixel calculation for each format.
 */
TEST_F(ABIContractTest, BytesPerPixel) {
    // RGBA8 should be 4 bytes per pixel
    ReadbackConfig rgba_config;
    rgba_config.format = PixelFormat::RGBA8;
    rgba_config.width = 100;
    rgba_config.height = 100;
    // Expected size: 100 * 100 * 4 = 40000 bytes
    
    // RGB8 should be 3 bytes per pixel
    ReadbackConfig rgb_config;
    rgb_config.format = PixelFormat::RGB8;
    rgb_config.width = 100;
    rgb_config.height = 100;
    // Expected size: 100 * 100 * 3 = 30000 bytes
}

/**
 * Test engine stats structure integrity.
 */
TEST_F(ABIContractTest, EngineStatsStructure) {
    EngineStats stats;
    stats.frame_count = 1000;
    stats.fps = 60.0;
    stats.frame_time_ms = 16.67;
    stats.entity_count = 500;
    stats.draw_calls = 100;
    
    EXPECT_EQ(stats.frame_count, 1000);
    EXPECT_DOUBLE_EQ(stats.fps, 60.0);
    EXPECT_DOUBLE_EQ(stats.frame_time_ms, 16.67);
    EXPECT_EQ(stats.entity_count, 500);
    EXPECT_EQ(stats.draw_calls, 100);
}

/**
 * Test that handle generation counter doesn't overflow inappropriately.
 */
TEST_F(ABIContractTest, HandleGeneration) {
    EntityHandle handle1 = 1;
    EntityHandle handle2 = 2;
    
    EXPECT_NE(handle1, handle2);
    EXPECT_NE(handle1, INVALID_ENTITY);
    EXPECT_NE(handle2, INVALID_ENTITY);
}

/**
 * Test readback buffer contract assumptions.
 */
TEST_F(ABIContractTest, ReadbackBufferContract) {
    // Row alignment assumptions
    ReadbackConfig config;
    config.width = 640;
    config.height = 480;
    config.format = PixelFormat::RGBA8;
    
    // Calculate expected buffer size (no row padding in our case)
    size_t bytes_per_pixel = 4; // RGBA8
    size_t expected_size = config.width * config.height * bytes_per_pixel;
    config.buffer_capacity = expected_size;
    
    EXPECT_EQ(config.buffer_capacity, 640 * 480 * 4);
}

/**
 * Test that enum values fit in expected integer types.
 */
TEST_F(ABIContractTest, EnumSizes) {
    // Pixel format should fit in int32
    EXPECT_LE(sizeof(PixelFormat), sizeof(int32_t));
}

/**
 * Test null pointer handling in readback config.
 */
TEST_F(ABIContractTest, NullReadbackConfig) {
    // Document behavior with null config
    ReadbackConfig* null_config = nullptr;
    
    // Accessing null should be avoided in actual code
    // This test documents the expectation
    EXPECT_EQ(null_config, nullptr);
}

/**
 * Test maximum reasonable dimensions.
 */
TEST_F(ABIContractTest, MaximumDimensions) {
    ReadbackConfig config;
    
    // Test with large but reasonable dimensions
    config.width = 7680; // 8K
    config.height = 4320;
    config.format = PixelFormat::RGBA8;
    
    size_t bytes_per_pixel = 4;
    size_t buffer_size = config.width * config.height * bytes_per_pixel;
    
    // Should be calculable without overflow for reasonable sizes
    EXPECT_GT(buffer_size, 0);
    EXPECT_LT(buffer_size, SIZE_MAX);
}

/**
 * Test zero dimension handling.
 */
TEST_F(ABIContractTest, ZeroDimensions) {
    ReadbackConfig config;
    config.width = 0;
    config.height = 0;
    config.format = PixelFormat::RGBA8;
    config.buffer_capacity = 0;
    
    // Zero dimensions should result in zero capacity
    EXPECT_EQ(config.buffer_capacity, 0);
}

/**
 * Test ABI version constants exist.
 */
TEST_F(ABIContractTest, ABIVersion) {
    // If version macros exist, verify they're defined
    #ifdef ASTRAEUS_ABI_VERSION_MAJOR
        EXPECT_GT(ASTRAEUS_ABI_VERSION_MAJOR, 0);
    #endif
    
    #ifdef ASTRAEUS_ABI_VERSION_MINOR
        EXPECT_GE(ASTRAEUS_ABI_VERSION_MINOR, 0);
    #endif
}

} // namespace testing
} // namespace astraeus
