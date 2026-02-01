#ifndef ASTRAEUS_GAMMA_CORRECTION_PASS_HPP
#define ASTRAEUS_GAMMA_CORRECTION_PASS_HPP

#include "PostProcessPass.hpp"
#include <string>
#include <iostream>

namespace astraeus {

/**
 * GammaCorrectionPass: Applies gamma correction to linearize or de-linearize colors.
 * Typically used as the final step to convert from linear to sRGB space.
 */
class GammaCorrectionPass : public PostProcessPass {
public:
    inline GammaCorrectionPass();
    inline ~GammaCorrectionPass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void apply(uint32_t input_texture, uint32_t output_fbo) override;
    inline const char* get_name() const override { return "GammaCorrection"; }

    // Configuration
    inline void set_gamma(float gamma);
    inline float get_gamma() const { return gamma_; }

private:
    inline void compile_shader();
    inline void destroy_shader();

    GLRenderDevice::ShaderHandle shader_;
    float gamma_; // Typical values: 2.2 (sRGB), 1.0 (linear), etc.
};

// Vertex shader for gamma correction (simple pass-through)
inline constexpr const char* gamma_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment shader for gamma correction
inline constexpr const char* gamma_fragment_shader = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uInputTexture;
uniform float uGamma;

void main() {
    vec4 color = texture(uInputTexture, TexCoord);
    
    // Apply gamma correction to RGB channels only
    // Note: For optimal performance, consider using shader variants for different gamma values.
    // However, the branch prediction cost is minimal for this simple comparison,
    // and this approach provides flexibility without shader recompilation.
    vec3 corrected;
    if (abs(uGamma - 2.2) < 0.01) {
        // Fast sRGB approximation
        corrected = pow(color.rgb, vec3(1.0 / 2.2));
    } else if (abs(uGamma - 1.0) < 0.01) {
        // Linear (no correction)
        corrected = color.rgb;
    } else {
        // Generic gamma correction
        corrected = pow(color.rgb, vec3(1.0 / uGamma));
    }
    
    FragColor = vec4(corrected, color.a);
}
)";

// ============================================================================
// Inline implementations
// ============================================================================

inline GammaCorrectionPass::GammaCorrectionPass()
    : PostProcessPass()
    , gamma_(2.2f) // Default to sRGB gamma
{
}

inline GammaCorrectionPass::~GammaCorrectionPass() {
    destroy_shader();
}

inline bool GammaCorrectionPass::initialize(RenderDevice* device) {
    if (!PostProcessPass::initialize(device)) {
        return false;
    }
    
    compile_shader();
    return shader_.gl_program != 0;
}

inline void GammaCorrectionPass::compile_shader() {
    std::string error_msg;
    shader_ = gl_device_->create_shader(
        gamma_vertex_shader,
        gamma_fragment_shader,
        error_msg
    );
    
    if (shader_.gl_program == 0) {
        std::cerr << "[GammaCorrectionPass] Failed to compile shader: " << error_msg << std::endl;
    } else {
        // Cache uniform locations
        shader_.uniform_locations["uInputTexture"] = glGetUniformLocation(shader_.gl_program, "uInputTexture");
        shader_.uniform_locations["uGamma"] = glGetUniformLocation(shader_.gl_program, "uGamma");
    }
}

inline void GammaCorrectionPass::destroy_shader() {
    if (shader_.gl_program != 0) {
        gl_device_->destroy_shader(shader_);
        shader_.gl_program = 0;
    }
}

inline void GammaCorrectionPass::apply(uint32_t input_texture, uint32_t output_fbo) {
    if (!enabled_ || shader_.gl_program == 0) {
        return;
    }
    
    gl_device_->push_debug_group("GammaCorrectionPass");
    
    // Bind output framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
    
    // Disable depth test for post-processing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    // Bind shader and set uniforms
    glUseProgram(shader_.gl_program);
    
    // Bind input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input_texture);
    
    // Use cached uniform locations
    auto it_tex = shader_.uniform_locations.find("uInputTexture");
    if (it_tex != shader_.uniform_locations.end() && it_tex->second >= 0) {
        glUniform1i(it_tex->second, 0);
    }
    
    auto it_gamma = shader_.uniform_locations.find("uGamma");
    if (it_gamma != shader_.uniform_locations.end() && it_gamma->second >= 0) {
        glUniform1f(it_gamma->second, gamma_);
    }
    
    // Draw full-screen quad
    draw_fullscreen_quad();
    
    // Cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    
    gl_device_->pop_debug_group();
}

inline void GammaCorrectionPass::set_gamma(float gamma) {
    gamma_ = gamma;
}

} // namespace astraeus

#endif // ASTRAEUS_GAMMA_CORRECTION_PASS_HPP
