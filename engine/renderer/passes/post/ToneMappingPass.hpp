#ifndef ASTRAEUS_TONE_MAPPING_PASS_HPP
#define ASTRAEUS_TONE_MAPPING_PASS_HPP

#include "PostProcessPass.hpp"
#include <string>
#include <iostream>

namespace astraeus {

/**
 * ToneMappingPass: Applies tone mapping to HDR color values.
 * Supports multiple tone mapping operators (Reinhard, ACES, etc.)
 */
class ToneMappingPass : public PostProcessPass {
public:
    enum class ToneMapOperator {
        None = 0,       // Pass-through (no tone mapping)
        Reinhard = 1,   // Simple Reinhard tone mapping
        ReinhardLum = 2,// Reinhard with luminance preservation
        ACES = 3        // ACES filmic tone mapping
    };

    inline ToneMappingPass();
    inline ~ToneMappingPass();

    inline bool initialize(RenderDevice* device);
    inline void apply(uint32_t input_texture, uint32_t output_fbo);
    inline const char* get_name() const { return "ToneMapping"; }

    // Configuration
    inline void set_operator(ToneMapOperator op);
    inline void set_exposure(float exposure);
    
    inline ToneMapOperator get_operator() const { return operator_; }
    inline float get_exposure() const { return exposure_; }

private:
    inline void compile_shader();
    inline void destroy_shader();

    GLRenderDevice::ShaderHandle shader_;
    ToneMapOperator operator_;
    float exposure_;
};

// Vertex shader for tone mapping (simple pass-through)
inline constexpr const char* tonemap_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment shader for tone mapping
inline constexpr const char* tonemap_fragment_shader = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uInputTexture;
uniform float uExposure;
uniform int uOperator; // 0=None, 1=Reinhard, 2=ReinhardLum, 3=ACES

// Reinhard tone mapping
vec3 reinhardToneMap(vec3 color) {
    return color / (color + vec3(1.0));
}

// Reinhard tone mapping with luminance preservation
vec3 reinhardLumToneMap(vec3 color) {
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma / (1.0 + luma);
    return color * (toneMappedLuma / luma);
}

// ACES filmic tone mapping (approximation)
vec3 acesToneMap(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main() {
    vec3 color = texture(uInputTexture, TexCoord).rgb;
    
    // Apply exposure
    color *= uExposure;
    
    // Apply tone mapping
    if (uOperator == 1) {
        color = reinhardToneMap(color);
    } else if (uOperator == 2) {
        color = reinhardLumToneMap(color);
    } else if (uOperator == 3) {
        color = acesToneMap(color);
    }
    // else: pass-through (no tone mapping)
    
    FragColor = vec4(color, 1.0);
}
)";

// ============================================================================
// Inline implementations
// ============================================================================

inline ToneMappingPass::ToneMappingPass()
    : PostProcessPass()
    , operator_(ToneMapOperator::None)
    , exposure_(1.0f)
{
}

inline ToneMappingPass::~ToneMappingPass() {
    destroy_shader();
}

inline bool ToneMappingPass::initialize(RenderDevice* device) {
    if (!PostProcessPass::initialize(device)) {
        return false;
    }
    
    compile_shader();
    return shader_.gl_program != 0;
}

inline void ToneMappingPass::compile_shader() {
    std::string error_msg;
    shader_ = gl_device_->create_shader(
        tonemap_vertex_shader,
        tonemap_fragment_shader,
        error_msg
    );
    
    if (shader_.gl_program == 0) {
        std::cerr << "[ToneMappingPass] Failed to compile shader: " << error_msg << std::endl;
    } else {
        // Cache uniform locations
        shader_.uniform_locations["uInputTexture"] = glGetUniformLocation(shader_.gl_program, "uInputTexture");
        shader_.uniform_locations["uExposure"] = glGetUniformLocation(shader_.gl_program, "uExposure");
        shader_.uniform_locations["uOperator"] = glGetUniformLocation(shader_.gl_program, "uOperator");
    }
}

inline void ToneMappingPass::destroy_shader() {
    if (shader_.gl_program != 0) {
        gl_device_->destroy_shader(shader_);
        shader_.gl_program = 0;
    }
}

inline void ToneMappingPass::apply(uint32_t input_texture, uint32_t output_fbo) {
    if (!enabled_ || shader_.gl_program == 0) {
        return;
    }
    
    gl_device_->push_debug_group("ToneMappingPass");
    
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
    
    auto it_exp = shader_.uniform_locations.find("uExposure");
    if (it_exp != shader_.uniform_locations.end() && it_exp->second >= 0) {
        glUniform1f(it_exp->second, exposure_);
    }
    
    auto it_op = shader_.uniform_locations.find("uOperator");
    if (it_op != shader_.uniform_locations.end() && it_op->second >= 0) {
        glUniform1i(it_op->second, static_cast<int>(operator_));
    }
    
    // Draw full-screen quad
    draw_fullscreen_quad();
    
    // Cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    
    gl_device_->pop_debug_group();
}

inline void ToneMappingPass::set_operator(ToneMapOperator op) {
    operator_ = op;
}

inline void ToneMappingPass::set_exposure(float exposure) {
    exposure_ = exposure;
}

} // namespace astraeus

#endif // ASTRAEUS_TONE_MAPPING_PASS_HPP
