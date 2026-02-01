#include "RenderGraph.hpp"
#include "passes/post/PostChain.hpp"
#include "passes/post/ToneMappingPass.hpp"
#include "passes/post/GammaCorrectionPass.hpp"
#include "opengl/GLRenderDevice.hpp"
#include "core/Telemetry.hpp"
#include <iostream>

namespace astraeus {

RenderGraph::RenderGraph(RenderDevice* device, World* world, Telemetry* telemetry)
    : device_(device)
    , world_(world)
    , telemetry_(telemetry)
    , is_initialized_(false)
    , post_chain_enabled_(false)
{
}

RenderGraph::~RenderGraph() {
    shutdown();
}

bool RenderGraph::initialize() {
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

void RenderGraph::shutdown() {
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

void RenderGraph::execute() {
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
            // Note: dynamic_cast required because get_color_texture/get_main_fbo
            // are OpenGL-specific methods not in base RenderDevice interface.
            // PostChain requires OpenGL functionality (framebuffers, textures).
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

void RenderGraph::on_resize(uint32_t width, uint32_t height) {
    for (auto& pass : passes_) {
        pass->on_resize(width, height);
    }
    
    // Resize post-chain
    if (post_chain_) {
        post_chain_->on_resize(width, height);
    }
}

void RenderGraph::add_pass(std::unique_ptr<RenderPass> pass) {
    if (pass && pass->initialize(device_)) {
        passes_.push_back(std::move(pass));
    }
}

PostChain* RenderGraph::get_post_chain() const {
    return post_chain_.get();
}

void RenderGraph::set_post_chain_enabled(bool enabled) {
    post_chain_enabled_ = enabled;
    if (enabled) {
        ensure_post_chain_initialized();
    }
}

bool RenderGraph::is_post_chain_enabled() const {
    return post_chain_enabled_;
}

void RenderGraph::ensure_post_chain_initialized() {
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
        // This provides the output contract: linear HDR -> sRGB LDR
        // 
        // IMPORTANT: Passes are enabled by default. When PostChain itself is enabled,
        // these passes will execute. To disable individual passes, call:
        //   get_post_chain()->get_pass(index)->set_enabled(false)
        
        // Tone mapping: pass-through by default (ToneMapOperator::None)
        // Can be changed to Reinhard, ACES, etc. for HDR content
        // Note: 'None' means pass-through (identity operation), not an error state
        auto tone_map = std::make_unique<ToneMappingPass>();
        tone_map->set_operator(ToneMappingPass::ToneMapOperator::None);  // Pass-through mode
        tone_map->set_exposure(1.0f);
        post_chain_->add_pass(std::move(tone_map));
        
        // Gamma correction: converts linear to sRGB (gamma 2.2)
        // CRITICAL: Framebuffer is linear (GL_RGBA8), this is the ONLY gamma correction
        // to avoid double-sRGB issues. When PostChain is disabled, output is linear.
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
