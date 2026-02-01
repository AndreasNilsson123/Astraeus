#include <gtest/gtest.h>
#include "renderer/RenderGraph.hpp"
#include "tests/mocks/MockRenderDevice.hpp"
#include "scene/World.hpp"

namespace astraeus {
namespace testing {

class RenderPassTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_device_ = std::make_unique<MockRenderDevice>();
        world_ = std::make_unique<World>();
    }
    
    void TearDown() override {
        world_.reset();
        mock_device_.reset();
    }
    
    std::unique_ptr<MockRenderDevice> mock_device_;
    std::unique_ptr<World> world_;
};

/**
 * Test RenderPass interface compliance.
 */
TEST_F(RenderPassTest, InterfaceCompliance) {
    class MinimalPass : public RenderPass {
    public:
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override {}
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "MinimalPass"; }
    };
    
    std::unique_ptr<RenderPass> pass = std::make_unique<MinimalPass>();
    
    mock_device_->initialize();
    
    EXPECT_TRUE(pass->initialize(mock_device_.get()));
    ASSERT_NO_THROW({
        pass->execute(mock_device_.get(), world_.get());
    });
    ASSERT_NO_THROW({
        pass->on_resize(800, 600);
    });
    EXPECT_STREQ(pass->get_name(), "MinimalPass");
}

/**
 * Test pass that accesses device during execution.
 */
TEST_F(RenderPassTest, DeviceInteraction) {
    class DeviceUsingPass : public RenderPass {
    public:
        bool initialize(RenderDevice* device) override {
            device_ = device;
            return device_ != nullptr;
        }
        
        void execute(RenderDevice* device, World*) override {
            // Use device to create resources
            if (device) {
                device->set_viewport(0, 0, 800, 600);
                device->clear(0, 0, 0, 1);
            }
        }
        
        void on_resize(uint32_t w, uint32_t h) override {
            width_ = w;
            height_ = h;
        }
        
        const char* get_name() const override { return "DevicePass"; }
        
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        RenderDevice* device_ = nullptr;
    };
    
    mock_device_->initialize();
    
    auto pass = std::make_unique<DeviceUsingPass>();
    EXPECT_TRUE(pass->initialize(mock_device_.get()));
    
    mock_device_->clear_operations();
    pass->execute(mock_device_.get(), world_.get());
    
    // Verify device was used
    EXPECT_GT(mock_device_->operation_count(), 0);
}

/**
 * Test pass that queries world state.
 */
TEST_F(RenderPassTest, WorldQuery) {
    class WorldQueryPass : public RenderPass {
    public:
        mutable size_t entity_count = 0;
        
        bool initialize(RenderDevice*) override { return true; }
        
        void execute(RenderDevice*, World* world) override {
            if (world) {
                entity_count = world->get_entity_count();
            }
        }
        
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "WorldQueryPass"; }
    };
    
    // Add some entities to world
    for (int i = 0; i < 5; ++i) {
        world_->create_entity();
    }
    
    mock_device_->initialize();
    
    auto pass = std::make_unique<WorldQueryPass>();
    pass->initialize(mock_device_.get());
    pass->execute(mock_device_.get(), world_.get());
    
    EXPECT_EQ(pass->entity_count, 5);
}

/**
 * Test pass resize handling.
 */
TEST_F(RenderPassTest, ResizeHandling) {
    class ResizablePass : public RenderPass {
    public:
        mutable std::vector<std::pair<uint32_t, uint32_t>> resize_calls;
        
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override {}
        
        void on_resize(uint32_t w, uint32_t h) override {
            resize_calls.push_back({w, h});
        }
        
        const char* get_name() const override { return "ResizablePass"; }
    };
    
    auto pass = std::make_unique<ResizablePass>();
    
    pass->on_resize(800, 600);
    pass->on_resize(1920, 1080);
    pass->on_resize(640, 480);
    
    ASSERT_EQ(pass->resize_calls.size(), 3);
    EXPECT_EQ(pass->resize_calls[0].first, 800);
    EXPECT_EQ(pass->resize_calls[1].first, 1920);
    EXPECT_EQ(pass->resize_calls[2].first, 640);
}

/**
 * Test pass with initialization failure.
 */
TEST_F(RenderPassTest, InitializationFailure) {
    class FailingPass : public RenderPass {
    public:
        bool initialize(RenderDevice*) override {
            return false; // Simulate failure
        }
        void execute(RenderDevice*, World*) override {}
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "FailingPass"; }
    };
    
    mock_device_->initialize();
    
    auto pass = std::make_unique<FailingPass>();
    EXPECT_FALSE(pass->initialize(mock_device_.get()));
}

/**
 * Test pass name consistency.
 */
TEST_F(RenderPassTest, NameConsistency) {
    class NamedPass : public RenderPass {
    public:
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override {}
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "TestPassName"; }
    };
    
    auto pass = std::make_unique<NamedPass>();
    
    // Name should be consistent across calls
    const char* name1 = pass->get_name();
    const char* name2 = pass->get_name();
    
    EXPECT_STREQ(name1, name2);
    EXPECT_STREQ(name1, "TestPassName");
}

} // namespace testing
} // namespace astraeus
