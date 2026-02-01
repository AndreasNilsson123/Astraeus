#ifndef ASTRAEUS_BLOOM_PASS_HPP
#define ASTRAEUS_BLOOM_PASS_HPP

#include "PostProcessPass.hpp"
#include <string>
#include <iostream>

namespace astraeus {

/**
 * BloomPass: Extensible hook for bloom post-processing (ping-pong blur).
 * This is a stub implementation that provides the interface for future bloom support.
 * 
 * FUTURE IMPLEMENTATION NOTES:
 * - Extract bright pixels above threshold
 * - Perform Gaussian blur with ping-pong between two framebuffers
 * - Combine blurred result with original image
 */
class BloomPass : public PostProcessPass {
public:
    inline BloomPass();
    inline ~BloomPass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void apply(uint32_t input_texture, uint32_t output_fbo) override;
    inline const char* get_name() const override { return "Bloom"; }

    // Configuration (for future implementation)
    inline void set_threshold(float threshold);
    inline void set_intensity(float intensity);
    inline void set_blur_iterations(int iterations);
    
    inline float get_threshold() const { return threshold_; }
    inline float get_intensity() const { return intensity_; }
    inline int get_blur_iterations() const { return blur_iterations_; }

private:
    float threshold_;       // Brightness threshold for bloom
    float intensity_;       // Bloom intensity/strength
    int blur_iterations_;   // Number of blur passes (ping-pong)
    
    // Future: Framebuffers for ping-pong blur
    // uint32_t bright_fbo_;
    // uint32_t blur_fbo_[2];
    // GLRenderDevice::ShaderHandle extract_shader_;
    // GLRenderDevice::ShaderHandle blur_shader_;
    // GLRenderDevice::ShaderHandle combine_shader_;
};

// ============================================================================
// Inline implementations
// ============================================================================

inline BloomPass::BloomPass()
    : PostProcessPass()
    , threshold_(1.0f)
    , intensity_(0.5f)
    , blur_iterations_(5)
{
    // Bloom is disabled by default (stub implementation)
    enabled_ = false;
}

inline BloomPass::~BloomPass() {
    // Future: Cleanup framebuffers and shaders
}

inline bool BloomPass::initialize(RenderDevice* device) {
    if (!PostProcessPass::initialize(device)) {
        return false;
    }
    
    std::cout << "[BloomPass] Initialized (stub - not yet implemented)" << std::endl;
    
    // Future: Initialize framebuffers and compile shaders
    // - Create bright extraction FBO
    // - Create two FBOs for ping-pong blur
    // - Compile extract, blur, and combine shaders
    
    return true;
}

inline void BloomPass::apply(uint32_t input_texture, uint32_t output_fbo) {
    (void)input_texture;
    (void)output_fbo;
    
    if (!enabled_) {
        return;
    }
    
    // Stub implementation: Pass-through
    // Future implementation:
    // 1. Extract bright pixels (threshold)
    // 2. Blur bright pixels (ping-pong between two FBOs)
    // 3. Combine blurred result with original image
    
    std::cout << "[BloomPass] Apply called (stub - no effect)" << std::endl;
}

inline void BloomPass::set_threshold(float threshold) {
    threshold_ = threshold;
}

inline void BloomPass::set_intensity(float intensity) {
    intensity_ = intensity;
}

inline void BloomPass::set_blur_iterations(int iterations) {
    blur_iterations_ = iterations;
}

} // namespace astraeus

#endif // ASTRAEUS_BLOOM_PASS_HPP
