#ifndef ASTRAEUS_RENDER_GRAPH_HPP
#define ASTRAEUS_RENDER_GRAPH_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

namespace astraeus {

class RenderDevice;
class World;
class RenderPass;
class Telemetry;
class PostChain;

/**
 * Render graph manages the execution of multiple render passes.
 * Passes can include: grid, tracks, volumes, symbols, overlays, etc.
 */
class RenderGraph {
public:
    RenderGraph(RenderDevice* device, World* world, Telemetry* telemetry);
    ~RenderGraph();

    bool initialize();
    void shutdown();

    /**
     * Execute all render passes.
     */
    void execute();

    /**
     * Handle viewport resize.
     */
    void on_resize(uint32_t width, uint32_t height);

    /**
     * Add a render pass to the graph.
     */
    void add_pass(std::unique_ptr<RenderPass> pass);

    /**
     * Get the post-processing chain.
     * @return Pointer to the PostChain, or nullptr if not initialized
     */
    PostChain* get_post_chain() const;

private:
    RenderDevice* device_;
    World* world_;
    Telemetry* telemetry_;
    std::vector<std::unique_ptr<RenderPass>> passes_;
    std::unique_ptr<PostChain> post_chain_;
    bool is_initialized_;
};

/**
 * Base class for all render passes.
 */
class RenderPass {
public:
    virtual ~RenderPass() = default;

    virtual bool initialize(RenderDevice* device) = 0;
    virtual void execute(RenderDevice* device, World* world) = 0;
    virtual void on_resize(uint32_t width, uint32_t height) = 0;
    
    // Get a human-readable name for this pass (for telemetry)
    virtual const char* get_name() const = 0;

protected:
    RenderPass() = default;
};

// ============================================================================
// Inline implementations
// ============================================================================

inline RenderGraph::RenderGraph(RenderDevice* device, World* world, Telemetry* telemetry)
    : device_(device)
    , world_(world)
    , telemetry_(telemetry)
    , is_initialized_(false)
{
}

inline RenderGraph::~RenderGraph() {
    shutdown();
}

inline bool RenderGraph::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[RenderGraph] Initializing render graph" << std::endl;

    // TODO: Initialize default passes (grid, tracks, etc.)
    // For now, no passes added
    
    // Note: PostChain is initialized lazily on first use to ensure device is ready

    is_initialized_ = true;
    return true;
}

inline void RenderGraph::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[RenderGraph] Shutting down" << std::endl;
    
    // Shutdown post-chain
    if (post_chain_) {
        post_chain_->shutdown();
        post_chain_.reset();
    }
    
    passes_.clear();
    is_initialized_ = false;
}

inline void RenderGraph::execute() {
    // Execute all passes in order with per-pass timing
    for (auto& pass : passes_) {
        if (telemetry_ && telemetry_->is_enabled()) {
            // Manual timing (PassTimer can't be used here due to header dependencies)
            uint32_t pass_index = telemetry_->begin_pass(pass->get_name());
            pass->execute(device_, world_);
            telemetry_->end_pass(pass_index);
        } else {
            // No telemetry overhead when disabled
            pass->execute(device_, world_);
        }
    }
}

inline void RenderGraph::on_resize(uint32_t width, uint32_t height) {
    for (auto& pass : passes_) {
        pass->on_resize(width, height);
    }
    
    // Resize post-chain
    if (post_chain_) {
        post_chain_->on_resize(width, height);
    }
}

inline void RenderGraph::add_pass(std::unique_ptr<RenderPass> pass) {
    if (pass && pass->initialize(device_)) {
        passes_.push_back(std::move(pass));
    }
}

inline PostChain* RenderGraph::get_post_chain() const {
    return post_chain_.get();
}

} // namespace astraeus

#endif // ASTRAEUS_RENDER_GRAPH_HPP
