#ifndef ASTRAEUS_UNLIT_MATERIAL_HPP
#define ASTRAEUS_UNLIT_MATERIAL_HPP

#include "Material.hpp"
#include "opengl/GLRenderDevice.hpp"
#include <iostream>
#include <cstring>
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

/**
 * UnlitMaterial - Simple material with base color and optional texture.
 * Includes PBR-ready parameter structure (metallic/roughness) for future use.
 */
class UnlitMaterial : public Material {
public:
    UnlitMaterial();
    ~UnlitMaterial() override;

    const char* get_name() const override { return "Unlit"; }
    
    bool initialize(RenderDevice* device) override;
    void shutdown() override;
    
    void bind(RenderDevice* device) override;
    void set_parameter(const char* name, const MaterialParameter& param) override;
    void apply_parameters(RenderDevice* device) override;
    
    const PipelineState& get_pipeline_state() const override { return pipeline_state_; }
    bool is_initialized() const override { return is_initialized_; }

private:
    bool compile_shader(GLRenderDevice* gl_device);
    void destroy_shader();
    
    GLRenderDevice* gl_device_;
    GLuint shader_program_;
    bool is_initialized_;
    
    PipelineState pipeline_state_;
    
    // Cached uniform locations
    struct UniformLocations {
        GLint mvp = -1;
        GLint base_color = -1;
        GLint use_texture = -1;
        GLint base_color_texture = -1;
        // PBR-ready parameters (unused in unlit but present for future)
        GLint metallic = -1;
        GLint roughness = -1;
    } uniforms_;
    
    // Current parameter values
    struct Parameters {
        float base_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        float metallic = 0.0f;
        float roughness = 1.0f;
        uint32_t base_color_texture = 0;
        bool use_texture = false;
        float mvp[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    } params_;
};

// ============================================================================
// Implementation
// ============================================================================

inline UnlitMaterial::UnlitMaterial()
    : gl_device_(nullptr)
    , shader_program_(0)
    , is_initialized_(false)
{
    // Configure default pipeline state for unlit rendering
    pipeline_state_.depth_test_enabled = true;
    pipeline_state_.depth_write_enabled = true;
    pipeline_state_.blend_enabled = false;
    pipeline_state_.cull_enabled = true;
    pipeline_state_.cull_face = GL_BACK;
    pipeline_state_.primitive_type = GL_TRIANGLES;
}

inline UnlitMaterial::~UnlitMaterial() {
    shutdown();
}

inline bool UnlitMaterial::initialize(RenderDevice* device) {
    if (is_initialized_) {
        return true;
    }

    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        std::cerr << "[UnlitMaterial] Failed to get GLRenderDevice" << std::endl;
        return false;
    }

    if (!compile_shader(gl_device_)) {
        std::cerr << "[UnlitMaterial] Failed to compile shader" << std::endl;
        return false;
    }

    is_initialized_ = true;
    std::cout << "[UnlitMaterial] Initialized successfully" << std::endl;
    return true;
}

inline void UnlitMaterial::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    destroy_shader();
    gl_device_ = nullptr;
    is_initialized_ = false;
}

inline void UnlitMaterial::bind(RenderDevice* device) {
    (void)device; // Unused in this simple implementation
    
    if (!is_initialized_ || shader_program_ == 0) {
        return;
    }

    glUseProgram(shader_program_);
    
    // Apply pipeline state
    if (pipeline_state_.depth_test_enabled) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(pipeline_state_.depth_func);
        if (pipeline_state_.depth_write_enabled) {
            glDepthMask(GL_TRUE);
        } else {
            glDepthMask(GL_FALSE);
        }
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    
    if (pipeline_state_.blend_enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(pipeline_state_.blend_src_factor, pipeline_state_.blend_dst_factor);
    } else {
        glDisable(GL_BLEND);
    }
    
    if (pipeline_state_.cull_enabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(pipeline_state_.cull_face);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

inline void UnlitMaterial::set_parameter(const char* name, const MaterialParameter& param) {
    std::string param_name(name);
    
    if (param_name == "baseColor" && param.type == MaterialParameterType::Vec4) {
        params_.base_color[0] = param.data.vec4_value[0];
        params_.base_color[1] = param.data.vec4_value[1];
        params_.base_color[2] = param.data.vec4_value[2];
        params_.base_color[3] = param.data.vec4_value[3];
    }
    else if (param_name == "baseColorTexture" && param.type == MaterialParameterType::Texture2D) {
        params_.base_color_texture = param.data.texture_id;
        params_.use_texture = (param.data.texture_id != 0);
    }
    else if (param_name == "metallic" && param.type == MaterialParameterType::Float) {
        params_.metallic = param.data.float_value;
    }
    else if (param_name == "roughness" && param.type == MaterialParameterType::Float) {
        params_.roughness = param.data.float_value;
    }
    else if (param_name == "mvp" && param.type == MaterialParameterType::Mat4) {
        std::memcpy(params_.mvp, param.data.mat4_value, sizeof(float) * 16);
    }
}

inline void UnlitMaterial::apply_parameters(RenderDevice* device) {
    (void)device; // Unused
    
    if (!is_initialized_ || shader_program_ == 0) {
        return;
    }

    // Apply all parameters to shader uniforms
    if (uniforms_.base_color >= 0) {
        glUniform4fv(uniforms_.base_color, 1, params_.base_color);
    }
    
    if (uniforms_.use_texture >= 0) {
        glUniform1i(uniforms_.use_texture, params_.use_texture ? 1 : 0);
    }
    
    if (uniforms_.mvp >= 0) {
        glUniformMatrix4fv(uniforms_.mvp, 1, GL_FALSE, params_.mvp);
    }
    
    // PBR-ready parameters (not used in unlit shader but set for consistency)
    if (uniforms_.metallic >= 0) {
        glUniform1f(uniforms_.metallic, params_.metallic);
    }
    
    if (uniforms_.roughness >= 0) {
        glUniform1f(uniforms_.roughness, params_.roughness);
    }
    
    // Bind texture if present
    if (params_.use_texture && params_.base_color_texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, params_.base_color_texture);
        if (uniforms_.base_color_texture >= 0) {
            glUniform1i(uniforms_.base_color_texture, 0);
        }
    }
}

inline bool UnlitMaterial::compile_shader([[maybe_unused]] GLRenderDevice* gl_device) {
    // Vertex shader with PBR-ready structure
    const char* vertex_source = R"(
        #version 330 core
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        
        uniform mat4 uMVP;
        
        out vec3 vNormal;
        out vec2 vTexCoord;
        
        void main() {
            gl_Position = uMVP * vec4(aPosition, 1.0);
            vNormal = aNormal;
            vTexCoord = aTexCoord;
        }
    )";

    // Fragment shader - unlit with optional texture, PBR-ready parameters
    const char* fragment_source = R"(
        #version 330 core
        in vec3 vNormal;
        in vec2 vTexCoord;
        
        uniform vec4 uBaseColor;
        uniform int uUseTexture;
        uniform sampler2D uBaseColorTexture;
        
        // PBR-ready parameters (unused in unlit but present for future)
        uniform float uMetallic;
        uniform float uRoughness;
        
        out vec4 FragColor;
        
        void main() {
            vec4 finalColor = uBaseColor;
            
            if (uUseTexture != 0) {
                vec4 texColor = texture(uBaseColorTexture, vTexCoord);
                finalColor = finalColor * texColor;
            }
            
            FragColor = finalColor;
        }
    )";

    // Create and compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
    glCompileShader(vertex_shader);

    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info_log[1024];
        glGetShaderInfoLog(vertex_shader, 1024, nullptr, info_log);
        std::cerr << "[UnlitMaterial] Vertex shader compilation failed:\n" << info_log << std::endl;
        glDeleteShader(vertex_shader);
        return false;
    }

    // Create and compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info_log[1024];
        glGetShaderInfoLog(fragment_shader, 1024, nullptr, info_log);
        std::cerr << "[UnlitMaterial] Fragment shader compilation failed:\n" << info_log << std::endl;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    // Link program
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glLinkProgram(shader_program_);

    glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar info_log[1024];
        glGetProgramInfoLog(shader_program_, 1024, nullptr, info_log);
        std::cerr << "[UnlitMaterial] Shader program linking failed:\n" << info_log << std::endl;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
        return false;
    }

    // Clean up shaders (no longer needed after linking)
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Get uniform locations
    uniforms_.mvp = glGetUniformLocation(shader_program_, "uMVP");
    uniforms_.base_color = glGetUniformLocation(shader_program_, "uBaseColor");
    uniforms_.use_texture = glGetUniformLocation(shader_program_, "uUseTexture");
    uniforms_.base_color_texture = glGetUniformLocation(shader_program_, "uBaseColorTexture");
    uniforms_.metallic = glGetUniformLocation(shader_program_, "uMetallic");
    uniforms_.roughness = glGetUniformLocation(shader_program_, "uRoughness");

    std::cout << "[UnlitMaterial] Shader compiled successfully" << std::endl;
    return true;
}

inline void UnlitMaterial::destroy_shader() {
    if (shader_program_ != 0) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
    }
}

} // namespace astraeus

#endif // ASTRAEUS_UNLIT_MATERIAL_HPP
