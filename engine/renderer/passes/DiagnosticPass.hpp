#ifndef ASTRAEUS_DIAGNOSTIC_PASS_HPP
#define ASTRAEUS_DIAGNOSTIC_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include <iostream>

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

/**
 * DiagnosticPass: Renders a fullscreen UV gradient to verify viewport coverage.
 * 
 * This pass is used for debugging viewport/scissor issues. It renders a quad
 * that covers the entire screen with a gradient based on UV coordinates:
 * - Red increases from left to right (U coordinate)
 * - Green increases from bottom to top (V coordinate)
 * 
 * If the entire screen shows the gradient, viewport/scissor are correctly configured.
 * If only part of the screen shows the gradient, there's a viewport/scissor issue.
 */
class DiagnosticPass : public RenderPass {
public:
    inline DiagnosticPass();
    inline ~DiagnosticPass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;
    inline const char* get_name() const override { return "Diagnostic"; }

    // Enable/disable the diagnostic overlay
    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool is_enabled() const { return enabled_; }

private:
    GLRenderDevice* gl_device_;
    GLRenderDevice::ShaderHandle shader_;
    uint32_t vao_;
    GLRenderDevice::BufferHandle vertex_buffer_;
    bool enabled_;
};

// Shaders for fullscreen UV gradient
inline constexpr const char* diagnostic_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

inline constexpr const char* diagnostic_fragment_shader = R"(
#version 330 core
in vec2 vUV;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint EntityID;

void main() {
    // UV gradient: red from left to right, green from bottom to top
    // Add a blue tint for visibility
    FragColor = vec4(vUV.x, vUV.y, 0.3, 0.5);
    EntityID = 0u;
}
)";

// Implementation

inline DiagnosticPass::DiagnosticPass()
    : gl_device_(nullptr)
    , vao_(0)
    , enabled_(false)  // Disabled by default
{
}

inline DiagnosticPass::~DiagnosticPass() {
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    if (vertex_buffer_.gl_id != 0 && gl_device_) {
        gl_device_->destroy_buffer(vertex_buffer_);
    }
    if (shader_.gl_program != 0 && gl_device_) {
        gl_device_->destroy_shader(shader_);
    }
}

inline bool DiagnosticPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    // Compile shader
    std::string error_msg;
    shader_ = gl_device_->create_shader(diagnostic_vertex_shader, 
                                       diagnostic_fragment_shader, 
                                       error_msg);
    if (shader_.gl_program == 0) {
        std::cerr << "[DiagnosticPass] Failed to create shader: " << error_msg << std::endl;
        return false;
    }

    gl_device_->set_object_label(GL_PROGRAM, shader_.gl_program, "DiagnosticShader");

    // Create fullscreen quad with UV coordinates
    // Position in NDC space (-1 to 1), UV from 0 to 1
    float vertices[] = {
        // Position (X, Y), UV (U, V)
        -1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
         1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
         1.0f,  1.0f,  1.0f, 1.0f,  // Top-right
        
        -1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
         1.0f,  1.0f,  1.0f, 1.0f,  // Top-right
        -1.0f,  1.0f,  0.0f, 1.0f   // Top-left
    };

    vertex_buffer_ = gl_device_->create_buffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
    gl_device_->set_object_label(GL_BUFFER, vertex_buffer_.gl_id, "DiagnosticVB");

    // Create VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.gl_id);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // UV attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::cout << "[DiagnosticPass] Initialized (disabled by default)" << std::endl;
    return true;
}

inline void DiagnosticPass::execute(RenderDevice* device, World* world) {
    (void)device;
    (void)world;

    if (!enabled_ || !gl_device_) {
        return;
    }

    gl_device_->push_debug_group("DiagnosticPass");

    // Render fullscreen quad with UV gradient
    gl_device_->bind_shader(shader_);
    
    // Disable depth test for overlay
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for semi-transparent overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    // Restore depth test
    glEnable(GL_DEPTH_TEST);

    gl_device_->pop_debug_group();
}

inline void DiagnosticPass::on_resize(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
    // Fullscreen quad doesn't need to change with viewport size
}

} // namespace astraeus

#endif // ASTRAEUS_DIAGNOSTIC_PASS_HPP
