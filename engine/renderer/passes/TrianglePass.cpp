#include "TrianglePass.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include <cmath>
#include <iostream>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

namespace astraeus {

// Simple vertex and fragment shaders for the triangle
static const char* vertex_shader_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 uTransform;

void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

static const char* fragment_shader_src = R"(
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

TrianglePass::TrianglePass()
    : gl_device_(nullptr)
    , vao_(0)
    , rotation_angle_(0.0f)
{
}

TrianglePass::~TrianglePass() {
    destroy_geometry();
}

bool TrianglePass::initialize(RenderDevice* device) {
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

void TrianglePass::execute(RenderDevice* device, World* world) {
    (void)device;
    (void)world;

    if (!gl_device_ || shader_.gl_program == 0) {
        return;
    }

    gl_device_->push_debug_group("TrianglePass");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Bind shader
    gl_device_->bind_shader(shader_);

    // Update rotation
    rotation_angle_ += 0.01f;
    if (rotation_angle_ > 6.28318530718f) { // 2*PI
        rotation_angle_ -= 6.28318530718f;
    }

    // Create simple rotation matrix
    float cos_angle = std::cos(rotation_angle_);
    float sin_angle = std::sin(rotation_angle_);
    
    float transform[16] = {
        cos_angle, sin_angle, 0.0f, 0.0f,
        -sin_angle, cos_angle, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Set uniforms
    gl_device_->set_uniform_mat4(shader_, "uTransform", transform);
    
    GLint entity_id_loc = glGetUniformLocation(shader_.gl_program, "uEntityID");
    glUniform1ui(entity_id_loc, 1); // Entity ID = 1 for the triangle

    // Draw triangle
    glBindVertexArray(vao_);
    gl_device_->draw_arrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);

    gl_device_->pop_debug_group();
}

void TrianglePass::on_resize(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
    // Nothing special to do on resize
}

void TrianglePass::create_geometry() {
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

void TrianglePass::destroy_geometry() {
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
