#include <gtest/gtest.h>
#include "core/EngineContext.hpp"

namespace astraeus {
namespace testing {

class EngineLifecycleTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default configuration
        config_.initial_width = 800;
        config_.initial_height = 600;
        config_.enable_validation = false;
        config_.enable_debug_output = false;
    }
    
    EngineContext::Config config_;
};

/**
 * Test basic engine creation and destruction without initialization.
 * Verifies that the engine can be created and destroyed without crashes.
 */
TEST_F(EngineLifecycleTest, CreateAndDestroy) {
    EngineContext* engine = nullptr;
    
    ASSERT_NO_THROW({
        engine = new EngineContext(config_);
    });
    
    ASSERT_NE(engine, nullptr);
    
    ASSERT_NO_THROW({
        delete engine;
    });
}

/**
 * Test basic initialization and shutdown cycle.
 * Verifies that the engine can be initialized and shut down properly.
 */
TEST_F(EngineLifecycleTest, InitializeAndShutdown) {
    EngineContext engine(config_);
    
    bool init_result = engine.initialize();
    EXPECT_TRUE(init_result) << "Engine initialization should succeed";
    
    ASSERT_NO_THROW({
        engine.shutdown();
    });
}

/**
 * Test repeated initialization and shutdown cycles.
 * Verifies that the engine can handle multiple init/shutdown cycles.
 */
TEST_F(EngineLifecycleTest, RepeatedInitShutdown) {
    EngineContext engine(config_);
    
    for (int i = 0; i < 3; ++i) {
        bool init_result = engine.initialize();
        EXPECT_TRUE(init_result) << "Engine initialization should succeed on cycle " << i;
        
        ASSERT_NO_THROW({
            engine.shutdown();
        });
    }
}

/**
 * Test that shutdown without initialization doesn't crash.
 * Verifies robustness against incorrect usage patterns.
 */
TEST_F(EngineLifecycleTest, ShutdownWithoutInit) {
    EngineContext engine(config_);
    
    ASSERT_NO_THROW({
        engine.shutdown();
    });
}

/**
 * Test frame lifecycle with begin/end frame calls.
 * Verifies basic frame processing without crashes.
 */
TEST_F(EngineLifecycleTest, FrameLifecycle) {
    EngineContext engine(config_);
    engine.initialize();
    
    // Process a few frames
    for (int i = 0; i < 5; ++i) {
        ASSERT_NO_THROW({
            engine.begin_frame(0.016); // ~60 FPS
            engine.end_frame();
        });
    }
    
    engine.shutdown();
}

/**
 * Test viewport resize handling.
 * Verifies that the engine can handle viewport changes.
 */
TEST_F(EngineLifecycleTest, ViewportResize) {
    EngineContext engine(config_);
    engine.initialize();
    
    ASSERT_NO_THROW({
        engine.resize_viewport(1920, 1080);
    });
    
    ASSERT_NO_THROW({
        engine.resize_viewport(640, 480);
    });
    
    engine.shutdown();
}

/**
 * Test viewport resize with zero dimensions.
 * Verifies handling of invalid inputs.
 */
TEST_F(EngineLifecycleTest, ViewportResizeZero) {
    EngineContext engine(config_);
    engine.initialize();
    
    // Should handle gracefully (either ignore or clamp)
    ASSERT_NO_THROW({
        engine.resize_viewport(0, 0);
    });
    
    engine.shutdown();
}

/**
 * Test memory leak by creating and destroying multiple engines.
 * Note: This test relies on external tools like Valgrind or ASan for validation.
 */
TEST_F(EngineLifecycleTest, NoMemoryLeaks) {
    for (int i = 0; i < 10; ++i) {
        EngineContext* engine = new EngineContext(config_);
        engine->initialize();
        
        // Process a frame
        engine->begin_frame(0.016);
        engine->end_frame();
        
        engine->shutdown();
        delete engine;
    }
}

/**
 * Test different configuration options.
 * Verifies that various configurations are accepted.
 */
TEST_F(EngineLifecycleTest, DifferentConfigurations) {
    // Small viewport
    {
        EngineContext::Config small_config = config_;
        small_config.initial_width = 320;
        small_config.initial_height = 240;
        
        EngineContext engine(small_config);
        EXPECT_TRUE(engine.initialize());
        engine.shutdown();
    }
    
    // Large viewport
    {
        EngineContext::Config large_config = config_;
        large_config.initial_width = 3840;
        large_config.initial_height = 2160;
        
        EngineContext engine(large_config);
        EXPECT_TRUE(engine.initialize());
        engine.shutdown();
    }
}

} // namespace testing
} // namespace astraeus
