#include <gtest/gtest.h>
#include "renderer/RenderGraph.hpp"
#include "tests/mocks/MockRenderDevice.hpp"
#include "scene/World.hpp"
#include "core/Telemetry.hpp"

namespace astraeus {
namespace testing {

class RenderGraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_device_ = std::make_unique<MockRenderDevice>();
        world_ = std::make_unique<World>();
        telemetry_ = std::make_unique<Telemetry>();
        
        render_graph_ = std::make_unique<RenderGraph>(
            mock_device_.get(),
            world_.get(),
            telemetry_.get()
        );
    }
    
    void TearDown() override {
        render_graph_.reset();
        telemetry_.reset();
        world_.reset();
        mock_device_.reset();
    }
    
    std::unique_ptr<MockRenderDevice> mock_device_;
    std::unique_ptr<World> world_;
    std::unique_ptr<Telemetry> telemetry_;
    std::unique_ptr<RenderGraph> render_graph_;
};

/**
 * Test basic RenderGraph initialization.
 */
TEST_F(RenderGraphTest, Initialize) {
    ASSERT_TRUE(mock_device_->initialize());
    ASSERT_TRUE(render_graph_->initialize());
}

/**
 * Test shutdown without initialization.
 */
TEST_F(RenderGraphTest, ShutdownWithoutInit) {
    ASSERT_NO_THROW({
        render_graph_->shutdown();
    });
}

/**
 * Test complete lifecycle.
 */
TEST_F(RenderGraphTest, CompleteLifecycle) {
    mock_device_->initialize();
    render_graph_->initialize();
    
    ASSERT_NO_THROW({
        render_graph_->shutdown();
    });
}

/**
 * Test execute without passes.
 */
TEST_F(RenderGraphTest, ExecuteEmpty) {
    mock_device_->initialize();
    render_graph_->initialize();
    
    ASSERT_NO_THROW({
        render_graph_->execute();
    });
    
    // Should not crash with no passes
}

/**
 * Test adding a render pass.
 */
TEST_F(RenderGraphTest, AddPass) {
    // Create a simple mock pass
    class TestPass : public RenderPass {
    public:
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override {}
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "TestPass"; }
    };
    
    mock_device_->initialize();
    render_graph_->initialize();
    
    ASSERT_NO_THROW({
        render_graph_->add_pass(std::make_unique<TestPass>());
    });
}

/**
 * Test executing with a single pass.
 */
TEST_F(RenderGraphTest, ExecuteWithSinglePass) {
    class TestPass : public RenderPass {
    public:
        mutable int execute_count = 0;
        
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override { execute_count++; }
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "TestPass"; }
    };
    
    mock_device_->initialize();
    render_graph_->initialize();
    
    auto* test_pass = new TestPass();
    render_graph_->add_pass(std::unique_ptr<RenderPass>(test_pass));
    
    render_graph_->execute();
    
    EXPECT_EQ(test_pass->execute_count, 1);
}

/**
 * Test pass execution order.
 */
TEST_F(RenderGraphTest, PassExecutionOrder) {
    std::vector<int> execution_order;
    
    class OrderedPass : public RenderPass {
    public:
        OrderedPass(int id, std::vector<int>* order) : id_(id), order_(order) {}
        
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override { order_->push_back(id_); }
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "OrderedPass"; }
        
    private:
        int id_;
        std::vector<int>* order_;
    };
    
    mock_device_->initialize();
    render_graph_->initialize();
    
    render_graph_->add_pass(std::make_unique<OrderedPass>(1, &execution_order));
    render_graph_->add_pass(std::make_unique<OrderedPass>(2, &execution_order));
    render_graph_->add_pass(std::make_unique<OrderedPass>(3, &execution_order));
    
    render_graph_->execute();
    
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 3);
}

/**
 * Test viewport resize propagation.
 */
TEST_F(RenderGraphTest, ViewportResize) {
    class ResizeTrackingPass : public RenderPass {
    public:
        mutable uint32_t last_width = 0;
        mutable uint32_t last_height = 0;
        
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override {}
        void on_resize(uint32_t w, uint32_t h) override {
            last_width = w;
            last_height = h;
        }
        const char* get_name() const override { return "ResizePass"; }
    };
    
    mock_device_->initialize();
    render_graph_->initialize();
    
    auto* resize_pass = new ResizeTrackingPass();
    render_graph_->add_pass(std::unique_ptr<RenderPass>(resize_pass));
    
    render_graph_->on_resize(1920, 1080);
    
    EXPECT_EQ(resize_pass->last_width, 1920);
    EXPECT_EQ(resize_pass->last_height, 1080);
}

/**
 * Test multiple executions.
 */
TEST_F(RenderGraphTest, MultipleExecutions) {
    class CountingPass : public RenderPass {
    public:
        mutable int count = 0;
        
        bool initialize(RenderDevice*) override { return true; }
        void execute(RenderDevice*, World*) override { count++; }
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "CountingPass"; }
    };
    
    mock_device_->initialize();
    render_graph_->initialize();
    
    auto* counting_pass = new CountingPass();
    render_graph_->add_pass(std::unique_ptr<RenderPass>(counting_pass));
    
    for (int i = 0; i < 10; ++i) {
        render_graph_->execute();
    }
    
    EXPECT_EQ(counting_pass->count, 10);
}

/**
 * Test post-processing chain enable/disable.
 */
TEST_F(RenderGraphTest, PostProcessingChain) {
    mock_device_->initialize();
    render_graph_->initialize();
    
    EXPECT_FALSE(render_graph_->is_post_chain_enabled());
    
    render_graph_->set_post_chain_enabled(true);
    EXPECT_TRUE(render_graph_->is_post_chain_enabled());
    
    render_graph_->set_post_chain_enabled(false);
    EXPECT_FALSE(render_graph_->is_post_chain_enabled());
}

/**
 * Test pass with failed initialization.
 */
TEST_F(RenderGraphTest, FailedPassInitialization) {
    class FailingPass : public RenderPass {
    public:
        bool initialize(RenderDevice*) override { return false; }
        void execute(RenderDevice*, World*) override {}
        void on_resize(uint32_t, uint32_t) override {}
        const char* get_name() const override { return "FailingPass"; }
    };
    
    mock_device_->initialize();
    render_graph_->initialize();
    
    // Should handle failed pass initialization gracefully
    ASSERT_NO_THROW({
        render_graph_->add_pass(std::make_unique<FailingPass>());
    });
}

} // namespace testing
} // namespace astraeus
