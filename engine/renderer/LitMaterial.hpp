#ifndef ASTRAEUS_LIT_MATERIAL_HPP
#define ASTRAEUS_LIT_MATERIAL_HPP

#include "Material.hpp"
#include "opengl/GLRenderDevice.hpp"
#include <iostream>
#include <cstring>
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

/**
 * DirectionalLight - Directional light parameters
 * Used to pass light information to the lit material shader
 */
struct DirectionalLight {
    float direction[3];  // Light direction (normalized)
    float color[3];      // Light color (RGB)
    float intensity;     // Light intensity
    float ambient[3];    // Ambient light color (RGB)
    
    DirectionalLight()
        : direction{0.0f, -1.0f, 0.0f}  // Default: pointing down
        , color{1.0f, 1.0f, 1.0f}       // Default: white
        , intensity(1.0f)
        , ambient{0.2f, 0.2f, 0.2f}     // Default: 20% ambient
    {}
};

/**
 * LitMaterial - Material with Lambert diffuse + Blinn-Phong specular shading.
 * Supports directional light and ambient lighting.
 */
class LitMaterial : public Material {
public:
    LitMaterial();
    ~LitMaterial() override;

    const char* get_name() const override { return "Lit"; }
    
    bool initialize(RenderDevice* device) override;
    void shutdown() override;
    
    void bind(RenderDevice* device) override;
    void set_parameter(const char* name, const MaterialParameter& param) override;
    void apply_parameters(RenderDevice* device) override;
    
    const PipelineState& get_pipeline_state() const override { return pipeline_state_; }
    bool is_initialized() const override { return is_initialized_; }
    
    /**
     * Set directional light parameters
     */
    void set_directional_light(const DirectionalLight& light);

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
        GLint model = -1;
        GLint normal_matrix = -1;
        
        // Material properties
        GLint base_color = -1;
        GLint metallic = -1;
        GLint roughness = -1;
        GLint specular_strength = -1;
        
        // Lighting
        GLint light_direction = -1;
        GLint light_color = -1;
        GLint light_intensity = -1;
        GLint ambient_color = -1;
        
        // Camera
        GLint view_pos = -1;
        
        // Texture (optional)
        GLint use_texture = -1;
        GLint base_color_texture = -1;
    } uniforms_;
    
    // Current parameter values
    struct Parameters {
        float base_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        float metallic = 0.0f;
        float roughness = 0.5f;
        float specular_strength = 0.5f;
        uint32_t base_color_texture = 0;
        bool use_texture = false;
        float mvp[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        float model[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        float normal_matrix[9] = {1,0,0, 0,1,0, 0,0,1};
        float view_pos[3] = {0.0f, 0.0f, 0.0f};
    } params_;
    
    // Light parameters
    DirectionalLight light_;
};

// ============================================================================
// Implementation
// ============================================================================

inline LitMaterial::LitMaterial()
    : gl_device_(nullptr)
    , shader_program_(0)
    , is_initialized_(false)
{
    // Configure default pipeline state
    pipeline_state_.depth_test_enabled = true;
    pipeline_state_.depth_write_enabled = true;
    pipeline_state_.blend_enabled = false;
    pipeline_state_.cull_enabled = true;
    pipeline_state_.cull_face = GL_BACK;
    pipeline_state_.primitive_type = GL_TRIANGLES;
}

inline LitMaterial::~LitMaterial() {
    shutdown();
}

inline bool LitMaterial::initialize(RenderDevice* device) {
    if (is_initialized_) {
        return true;
    }

    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        std::cerr << "[LitMaterial] Failed to get GLRenderDevice" << std::endl;
        return false;
    }

    if (!compile_shader(gl_device_)) {
        std::cerr << "[LitMaterial] Failed to compile shader" << std::endl;
        return false;
    }

    is_initialized_ = true;
    std::cout << "[LitMaterial] Initialized successfully" << std::endl;
    return true;
}

inline void LitMaterial::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    destroy_shader();
    gl_device_ = nullptr;
    is_initialized_ = false;
}

inline void LitMaterial::bind(RenderDevice* device) {
    (void)device; // Unused
    
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

inline void LitMaterial::set_parameter(const char* name, const MaterialParameter& param) {
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
    else if (param_name == "specularStrength" && param.type == MaterialParameterType::Float) {
        params_.specular_strength = param.data.float_value;
    }
    else if (param_name == "mvp" && param.type == MaterialParameterType::Mat4) {
        std::memcpy(params_.mvp, param.data.mat4_value, sizeof(float) * 16);
    }
    else if (param_name == "model" && param.type == MaterialParameterType::Mat4) {
        std::memcpy(params_.model, param.data.mat4_value, sizeof(float) * 16);
    }
    else if (param_name == "viewPos" && param.type == MaterialParameterType::Vec3) {
        params_.view_pos[0] = param.data.vec3_value[0];
        params_.view_pos[1] = param.data.vec3_value[1];
        params_.view_pos[2] = param.data.vec3_value[2];
    }
}

inline void LitMaterial::set_directional_light(const DirectionalLight& light) {
    light_ = light;
}

inline void LitMaterial::apply_parameters(RenderDevice* device) {
    (void)device; // Unused
    
    if (!is_initialized_ || shader_program_ == 0) {
        return;
    }

    // Apply material parameters
    if (uniforms_.base_color >= 0) {
        glUniform4fv(uniforms_.base_color, 1, params_.base_color);
    }
    
    if (uniforms_.metallic >= 0) {
        glUniform1f(uniforms_.metallic, params_.metallic);
    }
    
    if (uniforms_.roughness >= 0) {
        glUniform1f(uniforms_.roughness, params_.roughness);
    }
    
    if (uniforms_.specular_strength >= 0) {
        glUniform1f(uniforms_.specular_strength, params_.specular_strength);
    }
    
    // Apply matrices
    if (uniforms_.mvp >= 0) {
        glUniformMatrix4fv(uniforms_.mvp, 1, GL_FALSE, params_.mvp);
    }
    
    if (uniforms_.model >= 0) {
        glUniformMatrix4fv(uniforms_.model, 1, GL_FALSE, params_.model);
    }
    
    if (uniforms_.normal_matrix >= 0) {
        glUniformMatrix3fv(uniforms_.normal_matrix, 1, GL_FALSE, params_.normal_matrix);
    }
    
    // Apply lighting parameters
    if (uniforms_.light_direction >= 0) {
        glUniform3fv(uniforms_.light_direction, 1, light_.direction);
    }
    
    if (uniforms_.light_color >= 0) {
        glUniform3fv(uniforms_.light_color, 1, light_.color);
    }
    
    if (uniforms_.light_intensity >= 0) {
        glUniform1f(uniforms_.light_intensity, light_.intensity);
    }
    
    if (uniforms_.ambient_color >= 0) {
        glUniform3fv(uniforms_.ambient_color, 1, light_.ambient);
    }
    
    // Apply camera position
    if (uniforms_.view_pos >= 0) {
        glUniform3fv(uniforms_.view_pos, 1, params_.view_pos);
    }
    
    // Apply texture settings
    if (uniforms_.use_texture >= 0) {
        glUniform1i(uniforms_.use_texture, params_.use_texture ? 1 : 0);
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

inline bool LitMaterial::compile_shader([[maybe_unused]] GLRenderDevice* gl_device) {
    // Vertex shader with lighting support
    const char* vertex_source = R"(
        #version 330 core
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        
        uniform mat4 uMVP;
        uniform mat4 uModel;
        uniform mat3 uNormalMatrix;
        
        out vec3 vWorldPos;
        out vec3 vNormal;
        out vec2 vTexCoord;
        
        void main() {
            // Transform position to world space
            vec4 worldPos = uModel * vec4(aPosition, 1.0);
            vWorldPos = worldPos.xyz;
            
            // Transform normal to world space
            vNormal = normalize(uNormalMatrix * aNormal);
            
            // Pass texture coordinates
            vTexCoord = aTexCoord;
            
            // Transform to clip space
            gl_Position = uMVP * vec4(aPosition, 1.0);
        }
    )";

    // Fragment shader with Lambert + Blinn-Phong lighting
    const char* fragment_source = R"(
        #version 330 core
        in vec3 vWorldPos;
        in vec3 vNormal;
        in vec2 vTexCoord;
        
        // Material properties
        uniform vec4 uBaseColor;
        uniform float uMetallic;
        uniform float uRoughness;
        uniform float uSpecularStrength;
        
        // Lighting
        uniform vec3 uLightDirection;
        uniform vec3 uLightColor;
        uniform float uLightIntensity;
        uniform vec3 uAmbientColor;
        
        // Camera
        uniform vec3 uViewPos;
        
        // Texture (optional)
        uniform int uUseTexture;
        uniform sampler2D uBaseColorTexture;
        
        out vec4 FragColor;
        
        void main() {
            // Get base color
            vec4 baseColor = uBaseColor;
            if (uUseTexture != 0) {
                vec4 texColor = texture(uBaseColorTexture, vTexCoord);
                baseColor = baseColor * texColor;
            }
            
            // Normalize vectors
            vec3 normal = normalize(vNormal);
            vec3 lightDir = normalize(-uLightDirection);  // Light points towards direction
            vec3 viewDir = normalize(uViewPos - vWorldPos);
            
            // Ambient component
            vec3 ambient = uAmbientColor * baseColor.rgb;
            
            // Diffuse component (Lambert)
            float diffuseStrength = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diffuseStrength * uLightColor * uLightIntensity * baseColor.rgb;
            
            // Specular component (Blinn-Phong)
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float specularPower = mix(2.0, 128.0, 1.0 - uRoughness);  // Map roughness to shininess
            float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), specularPower);
            vec3 specular = uSpecularStrength * specularStrength * uLightColor * uLightIntensity;
            
            // Combine lighting components
            vec3 finalColor = ambient + diffuse + specular;
            
            FragColor = vec4(finalColor, baseColor.a);
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
        std::cerr << "[LitMaterial] Vertex shader compilation failed:\n" << info_log << std::endl;
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
        std::cerr << "[LitMaterial] Fragment shader compilation failed:\n" << info_log << std::endl;
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
        std::cerr << "[LitMaterial] Shader program linking failed:\n" << info_log << std::endl;
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
    uniforms_.model = glGetUniformLocation(shader_program_, "uModel");
    uniforms_.normal_matrix = glGetUniformLocation(shader_program_, "uNormalMatrix");
    
    uniforms_.base_color = glGetUniformLocation(shader_program_, "uBaseColor");
    uniforms_.metallic = glGetUniformLocation(shader_program_, "uMetallic");
    uniforms_.roughness = glGetUniformLocation(shader_program_, "uRoughness");
    uniforms_.specular_strength = glGetUniformLocation(shader_program_, "uSpecularStrength");
    
    uniforms_.light_direction = glGetUniformLocation(shader_program_, "uLightDirection");
    uniforms_.light_color = glGetUniformLocation(shader_program_, "uLightColor");
    uniforms_.light_intensity = glGetUniformLocation(shader_program_, "uLightIntensity");
    uniforms_.ambient_color = glGetUniformLocation(shader_program_, "uAmbientColor");
    
    uniforms_.view_pos = glGetUniformLocation(shader_program_, "uViewPos");
    
    uniforms_.use_texture = glGetUniformLocation(shader_program_, "uUseTexture");
    uniforms_.base_color_texture = glGetUniformLocation(shader_program_, "uBaseColorTexture");

    std::cout << "[LitMaterial] Shader compiled successfully" << std::endl;
    return true;
}

inline void LitMaterial::destroy_shader() {
    if (shader_program_ != 0) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
    }
}

} // namespace astraeus

#endif // ASTRAEUS_LIT_MATERIAL_HPP
