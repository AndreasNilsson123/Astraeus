#ifndef ASTRAEUS_RENDER_GRAPH_HPP
#define ASTRAEUS_RENDER_GRAPH_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

namespace astraeus {

class RenderDevice;
class GLRenderDevice;
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

// Include post-processing passes for default initialization
#include "passes/post/PostChain.hpp"
#include "passes/post/ToneMappingPass.hpp"
#include "passes/post/GammaCorrectionPass.hpp"
#include "opengl/GLRenderDevice.hpp"

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
    // PostChain runs after main passes and before readback
    // Input: main color texture, Output: main FBO (in-place processing)
    if (post_chain_enabled_) {
        ensure_post_chain_initialized();
        if (post_chain_ && post_chain_->is_enabled()) {
            // Get color texture and main FBO from device
            GLRenderDevice* gl_device = dynamic_cast<GLRenderDevice*>(device_);
            if (gl_device) {
                uint32_t color_texture = gl_device->get_color_texture();
                uint32_t main_fbo = gl_device->get_main_fbo();
                
                // Apply post-chain with per-pass timing if telemetry enabled
                if (telemetry_ && telemetry_->is_enabled()) {
                    uint32_t pass_index = telemetry_->begin_pass("PostChain");
                    post_chain_->apply(color_texture, main_fbo);
                    telemetry_->end_pass(pass_index);
                } else {
                    post_chain_->apply(color_texture, main_fbo);
                }
            }
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
        uint32_t width = device_->get_width();
        uint32_t height = device_->get_height();
        
        // Validate dimensions before initializing
        if (width == 0 || height == 0) {
            std::cerr << "[RenderGraph] Cannot initialize PostChain: invalid viewport dimensions ("
                      << width << "x" << height << ")" << std::endl;
            return;
        }
        
        std::cout << "[RenderGraph] Initializing PostChain with default passes" << std::endl;
        post_chain_ = std::make_unique<PostChain>(device_);
        
        // Add default passes in stable order: tone-map -> gamma correction
        // These provide a baseline output contract (linear HDR -> sRGB LDR)
        // Note: Passes are disabled by default in PostChain, so they won't affect output
        // unless explicitly enabled via configuration
        
        // Tone mapping is disabled by default (ToneMapOperator::None)
        // This allows raw color output when post-processing is not desired
        auto tone_map = std::make_unique<ToneMappingPass>();
        tone_map->set_operator(ToneMappingPass::ToneMapOperator::None);  // Pass-through by default
        tone_map->set_exposure(1.0f);
        post_chain_->add_pass(std::move(tone_map));
        
        // Gamma correction is enabled by default to ensure proper sRGB output
        // This is critical for JavaFX readback compatibility
        auto gamma = std::make_unique<GammaCorrectionPass>();
        gamma->set_gamma(2.2f);  // Standard sRGB gamma
        post_chain_->add_pass(std::move(gamma));
        
        // Initialize the chain (allocates intermediate framebuffers)
        if (!post_chain_->initialize(width, height)) {
            std::cerr << "[RenderGraph] Failed to initialize PostChain" << std::endl;
            post_chain_.reset();
        }
    }
}

} // namespace astraeus

#endif // ASTRAEUS_RENDER_GRAPH_HPP
