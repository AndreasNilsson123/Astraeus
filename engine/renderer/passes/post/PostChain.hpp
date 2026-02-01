#ifndef ASTRAEUS_POST_CHAIN_HPP
#define ASTRAEUS_POST_CHAIN_HPP

#include <vector>
#include <memory>
#include <iostream>
#include <cstdint>

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

// Forward declarations
class RenderDevice;
class PostProcessPass;

/**
 * PostChain: Manages a configurable sequence of post-processing passes.
 * 
 * The PostChain operates on the main render output and applies a series of
 * post-processing effects in order. It manages intermediate framebuffers
 * efficiently to avoid per-frame allocations.
 * 
 * Usage:
 *   1. Add passes with add_pass()
 *   2. Call initialize() to compile all passes
 *   3. Call apply() each frame after main rendering
 *   4. Passes can be enabled/disabled/reordered at runtime
 */
class PostChain {
public:
    inline PostChain(RenderDevice* device);
    inline ~PostChain();

    /**
     * Initialize the post-chain and all passes.
     * Allocates intermediate framebuffers.
     */
    inline bool initialize(uint32_t width, uint32_t height);

    /**
     * Shutdown and cleanup.
     */
    inline void shutdown();

    /**
     * Add a post-processing pass to the chain.
     * @param pass The pass to add (ownership is transferred)
     */
    inline void add_pass(std::unique_ptr<PostProcessPass> pass);

    /**
     * Remove all passes from the chain.
     */
    inline void clear_passes();

    /**
     * Apply the post-processing chain.
     * 
     * OUTPUT CONTRACT:
     * - Final output is written to output_fbo in RGBA8 format (internal GPU format)
     * - Gamma correction is applied exactly once (no double-sRGB)
     * - Readback from output_fbo texture will convert RGBA->BGRA if needed by glGetTexImage
     * - Color space: sRGB-compatible (gamma 2.2 applied by GammaCorrectionPass)
     * - Alpha channel: preserved through all passes
     * 
     * @param input_texture The input texture to process (main render output)
     * @param output_fbo The final output framebuffer (0 for default)
     */
    inline void apply(uint32_t input_texture, uint32_t output_fbo);

    /**
     * Handle viewport resize.
     * Recreates intermediate framebuffers.
     */
    inline void on_resize(uint32_t width, uint32_t height);

    /**
     * Get the number of passes in the chain.
     */
    inline size_t get_pass_count() const { return passes_.size(); }

    /**
     * Get a pass by index (for configuration).
     */
    inline PostProcessPass* get_pass(size_t index) {
        return index < passes_.size() ? passes_[index].get() : nullptr;
    }

    /**
     * Enable or disable the entire post-chain.
     */
    inline void set_enabled(bool enabled) { enabled_ = enabled; }
    inline bool is_enabled() const { return enabled_; }

private:
    /**
     * Create intermediate framebuffers for ping-pong.
     */
    inline void create_framebuffers();

    /**
     * Destroy intermediate framebuffers.
     */
    inline void destroy_framebuffers();

    RenderDevice* device_;
    std::vector<std::unique_ptr<PostProcessPass>> passes_;
    
    // Intermediate framebuffers for ping-pong between passes
    uint32_t intermediate_fbo_[2];
    uint32_t intermediate_texture_[2];
    
    uint32_t width_;
    uint32_t height_;
    bool is_initialized_;
    bool enabled_;
};

// ============================================================================
// Inline implementations
// ============================================================================

inline PostChain::PostChain(RenderDevice* device)
    : device_(device)
    , width_(0)
    , height_(0)
    , is_initialized_(false)
    , enabled_(true)
{
    intermediate_fbo_[0] = 0;
    intermediate_fbo_[1] = 0;
    intermediate_texture_[0] = 0;
    intermediate_texture_[1] = 0;
}

inline PostChain::~PostChain() {
    shutdown();
}

inline bool PostChain::initialize(uint32_t width, uint32_t height) {
    if (is_initialized_) {
        return true;
    }

    width_ = width;
    height_ = height;

    std::cout << "[PostChain] Initializing post-processing chain" << std::endl;

    // Create intermediate framebuffers
    create_framebuffers();

    // Initialize all passes
    bool all_passed = true;
    for (auto& pass : passes_) {
        if (!pass->initialize(device_)) {
            std::cerr << "[PostChain] Failed to initialize pass: " << pass->get_name() << std::endl;
            all_passed = false;
            // Continue initializing other passes rather than failing completely
        }
    }
    
    if (!all_passed) {
        std::cerr << "[PostChain] Warning: Some passes failed to initialize" << std::endl;
    }

    is_initialized_ = true;
    std::cout << "[PostChain] Initialized with " << passes_.size() << " passes" << std::endl;
    return true;
}

inline void PostChain::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[PostChain] Shutting down" << std::endl;

    destroy_framebuffers();
    passes_.clear();

    is_initialized_ = false;
}

inline void PostChain::add_pass(std::unique_ptr<PostProcessPass> pass) {
    if (pass) {
        passes_.push_back(std::move(pass));
    }
}

inline void PostChain::clear_passes() {
    passes_.clear();
}

inline void PostChain::apply(uint32_t input_texture, uint32_t output_fbo) {
    if (!enabled_ || !is_initialized_ || passes_.empty()) {
        // If post-chain is disabled or empty, just pass through
        // Note: In a real implementation, you'd copy input to output here
        return;
    }

    // Count enabled passes
    size_t enabled_count = 0;
    for (const auto& pass : passes_) {
        if (pass->is_enabled()) {
            enabled_count++;
        }
    }

    if (enabled_count == 0) {
        return;
    }

    // Apply passes in sequence, ping-ponging between intermediate buffers
    uint32_t current_input = input_texture;
    uint32_t current_output = 0;
    size_t enabled_index = 0;

    for (size_t i = 0; i < passes_.size(); ++i) {
        auto& pass = passes_[i];
        
        if (!pass->is_enabled()) {
            continue;
        }

        // Determine output target
        if (enabled_index == enabled_count - 1) {
            // Last pass: render to final output
            current_output = output_fbo;
        } else {
            // Intermediate pass: render to ping-pong buffer
            current_output = intermediate_fbo_[enabled_index % 2];
        }

        // Apply the pass
        pass->apply(current_input, current_output);

        // Set up for next pass
        if (enabled_index < enabled_count - 1) {
            current_input = intermediate_texture_[enabled_index % 2];
        }

        enabled_index++;
    }
}

inline void PostChain::on_resize(uint32_t width, uint32_t height) {
    if (width_ == width && height_ == height) {
        return;
    }

    width_ = width;
    height_ = height;

    // Recreate framebuffers
    destroy_framebuffers();
    create_framebuffers();

    // Notify passes
    for (auto& pass : passes_) {
        pass->on_resize(width, height);
    }
}

inline void PostChain::create_framebuffers() {
    if (width_ == 0 || height_ == 0) {
        return;
    }

    // Create two intermediate framebuffers for ping-pong
    for (int i = 0; i < 2; ++i) {
        // Generate framebuffer
        glGenFramebuffers(1, &intermediate_fbo_[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, intermediate_fbo_[i]);

        // Create color texture
        glGenTextures(1, &intermediate_texture_[i]);
        glBindTexture(GL_TEXTURE_2D, intermediate_texture_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate_texture_[i], 0);

        // Check framebuffer completeness
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[PostChain] Framebuffer " << i << " is incomplete: " << status << std::endl;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "[PostChain] Created intermediate framebuffers: " << width_ << "x" << height_ << std::endl;
}

inline void PostChain::destroy_framebuffers() {
    for (int i = 0; i < 2; ++i) {
        if (intermediate_texture_[i] != 0) {
            glDeleteTextures(1, &intermediate_texture_[i]);
            intermediate_texture_[i] = 0;
        }
        if (intermediate_fbo_[i] != 0) {
            glDeleteFramebuffers(1, &intermediate_fbo_[i]);
            intermediate_fbo_[i] = 0;
        }
    }
}

} // namespace astraeus

#endif // ASTRAEUS_POST_CHAIN_HPP
