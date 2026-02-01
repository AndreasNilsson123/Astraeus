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
     * Enable or disable post-processing chain.
     * When enabled, PostChain will be initialized on next execute if not already.
     */
    void set_post_chain_enabled(bool enabled);
    
    /**
     * Check if post-processing is enabled.
     */
    bool is_post_chain_enabled() const;

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
    bool post_chain_enabled_;
    
    /**
     * Initialize post-chain if not already initialized.
     */
    void ensure_post_chain_initialized();
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
    , post_chain_enabled_(false)
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
    
    // Apply post-processing chain if enabled
    // NOTE: Post-processing is currently a no-op by default
    // To enable, call set_post_chain_enabled(true) and configure passes
    // via get_post_chain()->add_pass(...)
    if (post_chain_enabled_) {
        ensure_post_chain_initialized();
        if (post_chain_ && post_chain_->is_enabled()) {
            // Note: This is a placeholder for future integration
            // The PostChain needs access to the framebuffer's color texture
            // For now, PostChain is initialized but not automatically executed
            // to maintain compatibility with existing rendering pipeline
            // 
            // Future integration point:
            // 1. Get color texture from device
            // 2. Apply post-chain: post_chain_->apply(color_texture, main_fbo)
            // 3. Ensure readback compatibility
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

inline void RenderGraph::set_post_chain_enabled(bool enabled) {
    post_chain_enabled_ = enabled;
    if (enabled) {
        ensure_post_chain_initialized();
    }
}

inline bool RenderGraph::is_post_chain_enabled() const {
    return post_chain_enabled_;
}

inline void RenderGraph::ensure_post_chain_initialized() {
    if (!post_chain_ && device_) {
        std::cout << "[RenderGraph] Initializing PostChain" << std::endl;
        post_chain_ = std::make_unique<PostChain>(device_);
        
        uint32_t width = device_->get_width();
        uint32_t height = device_->get_height();
        
        if (!post_chain_->initialize(width, height)) {
            std::cerr << "[RenderGraph] Failed to initialize PostChain" << std::endl;
            post_chain_.reset();
        }
    }
}

} // namespace astraeus

#endif // ASTRAEUS_RENDER_GRAPH_HPP
