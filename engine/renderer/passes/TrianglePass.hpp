#ifndef ASTRAEUS_TRIANGLE_PASS_HPP
#define ASTRAEUS_TRIANGLE_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include <cmath>
#include <iostream>

#define GL_GLEXT_PROTOTYPES
#include <glad/glad.h>

namespace astraeus {

/**
 * TrianglePass: Renders an animated colored triangle for testing.
 */
class TrianglePass : public RenderPass {
public:
    inline TrianglePass();
    inline ~TrianglePass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;

private:
    inline void create_geometry();
    inline void destroy_geometry();

    GLRenderDevice* gl_device_;
    GLRenderDevice::ShaderHandle shader_;
    GLRenderDevice::BufferHandle vertex_buffer_;
    uint32_t vao_;
    float rotation_angle_;
};

inline constexpr float TWO_PI = 6.28318530718f;

// Simple vertex and fragment shaders for the triangle
inline constexpr const char* vertex_shader_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform float uRotation;

void main() {
    // Apply 2D rotation in clip space
    float c = cos(uRotation);
    float s = sin(uRotation);
    mat2 rotation = mat2(c, s, -s, c);
    vec2 rotatedPos = rotation * aPos.xy;
    gl_Position = vec4(rotatedPos, 0.0, 1.0);
    vertexColor = aColor;
}
)";

inline constexpr const char* fragment_shader_src = R"(
#version 330 core
in vec3 vertexColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint EntityID;

uniform uint uEntityID;

void main() {
    FragColor = vec4(vertexColor, 1.0);
    EntityID = uEntityID;
}
)";

// Implementation

inline TrianglePass::TrianglePass()
    : gl_device_(nullptr)
    , vao_(0)
    , rotation_angle_(0.0f)
{
}

inline TrianglePass::~TrianglePass() {
    destroy_geometry();
}

inline bool TrianglePass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    // Compile shader
    std::string error_msg;
    shader_ = gl_device_->create_shader(vertex_shader_src, fragment_shader_src, error_msg);
    if (shader_.gl_program == 0) {
        std::cerr << "[TrianglePass] Failed to create shader: " << error_msg << std::endl;
        return false;
    }

    gl_device_->set_object_label(GL_PROGRAM, shader_.gl_program, "TriangleShader");

    // Create geometry
    create_geometry();

    std::cout << "[TrianglePass] Initialized successfully" << std::endl;
    return true;
}

inline void TrianglePass::execute(RenderDevice* device, World* world) {
    (void)device;
    (void)world;

    if (!gl_device_ || shader_.gl_program == 0) {
        return;
    }

    gl_device_->push_debug_group("TrianglePass");

    // Disable face culling
    glDisable(GL_CULL_FACE);
    
    // Disable depth testing for this simple test
    glDisable(GL_DEPTH_TEST);

    // Bind shader FIRST before setting uniforms
    gl_device_->bind_shader(shader_);
    
    // Clear any previous errors
    while (glGetError() != GL_NO_ERROR);

    // Update rotation
    rotation_angle_ += 0.02f;
    if (rotation_angle_ > TWO_PI) {
        rotation_angle_ -= TWO_PI;
    }

    // Set uniforms - check if they exist
    GLint rotation_loc = glGetUniformLocation(shader_.gl_program, "uRotation");
    if (rotation_loc != -1) {
        glUniform1f(rotation_loc, rotation_angle_);
    }
    
    GLint entity_id_loc = glGetUniformLocation(shader_.gl_program, "uEntityID");
    if (entity_id_loc != -1) {
        glUniform1ui(entity_id_loc, 1); // Entity ID = 1 for the triangle
    }

    // Draw triangle
    glBindVertexArray(vao_);
    gl_device_->draw_arrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    gl_device_->pop_debug_group();
}

inline void TrianglePass::on_resize(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
    // Nothing special to do on resize
}

inline void TrianglePass::create_geometry() {
    // Triangle vertices: position (x, y, z) + color (r, g, b)
    float vertices[] = {
        // Positions         // Colors
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top (red)
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Bottom left (green)
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // Bottom right (blue)
    };

    // Create vertex buffer
    vertex_buffer_ = gl_device_->create_buffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
    gl_device_->set_object_label(GL_BUFFER, vertex_buffer_.gl_id, "TriangleVBO");

    // Create VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.gl_id);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gl_device_->set_object_label(GL_VERTEX_ARRAY, vao_, "TriangleVAO");
}

inline void TrianglePass::destroy_geometry() {
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }

    if (vertex_buffer_.gl_id != 0) {
        gl_device_->destroy_buffer(vertex_buffer_);
    }

    if (shader_.gl_program != 0) {
        gl_device_->destroy_shader(shader_);
    }
}

} // namespace astraeus

#endif // ASTRAEUS_TRIANGLE_PASS_HPP
