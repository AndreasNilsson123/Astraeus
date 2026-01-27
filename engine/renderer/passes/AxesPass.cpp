#include "AxesPass.hpp"
#include "../../scene/World.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include <iostream>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

namespace astraeus {

// Axis color constants (RGB)
constexpr float X_AXIS_COLOR[3] = {1.0f, 0.0f, 0.0f}; // Red
constexpr float Y_AXIS_COLOR[3] = {0.0f, 1.0f, 0.0f}; // Green
constexpr float Z_AXIS_COLOR[3] = {0.0f, 0.0f, 1.0f}; // Blue

// Vertex and fragment shaders for the axes
static const char* axes_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 uViewProjection;

out vec3 vertexColor;

void main() {
    gl_Position = uViewProjection * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

static const char* axes_fragment_shader = R"(
#version 330 core
in vec3 vertexColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint EntityID;

void main() {
    FragColor = vec4(vertexColor, 1.0);
    EntityID = 0u; // Axes have no entity ID
}
)";

AxesPass::AxesPass()
    : gl_device_(nullptr)
    , vao_(0)
    , axis_length_(5.0f)
    , line_width_(2.0f)
    , viewport_width_(1280)
    , viewport_height_(720)
{
}

AxesPass::~AxesPass() {
    destroy_geometry();
}

bool AxesPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    // Compile shader
    std::string error_msg;
    shader_ = gl_device_->create_shader(axes_vertex_shader, axes_fragment_shader, error_msg);
    if (shader_.gl_program == 0) {
        std::cerr << "[AxesPass] Failed to create shader: " << error_msg << std::endl;
        return false;
    }

    gl_device_->set_object_label(GL_PROGRAM, shader_.gl_program, "AxesShader");

    // Create geometry
    create_geometry();

    std::cout << "[AxesPass] Initialized successfully" << std::endl;
    return true;
}

void AxesPass::execute(RenderDevice* device, World* world) {
    (void)device;

    if (!gl_device_ || shader_.gl_program == 0 || !world) {
        return;
    }

    gl_device_->push_debug_group("AxesPass");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Disable blending
    glDisable(GL_BLEND);

    // Disable face culling
    glDisable(GL_CULL_FACE);

    // Set line width
    glLineWidth(line_width_);

    // Bind shader
    gl_device_->bind_shader(shader_);

    // Get camera from world and update matrices
    float aspect = static_cast<float>(viewport_width_) / static_cast<float>(viewport_height_);
    world->update_camera(aspect);
    
    const Camera& camera = world->get_camera();
    const float* vp_matrix = camera.get_view_projection_matrix();

    // Set uniforms
    GLint vp_loc = glGetUniformLocation(shader_.gl_program, "uViewProjection");
    if (vp_loc != -1) {
        glUniformMatrix4fv(vp_loc, 1, GL_FALSE, vp_matrix);
    }

    // Draw axes (3 lines, 2 vertices each = 6 vertices total)
    glBindVertexArray(vao_);
    gl_device_->draw_arrays(GL_LINES, 0, 6);
    glBindVertexArray(0);

    // Reset line width
    glLineWidth(1.0f);

    gl_device_->pop_debug_group();
}

void AxesPass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

void AxesPass::create_geometry() {
    // Three axes from origin, each with its color
    // Format: position (x, y, z) + color (r, g, b)
    float vertices[] = {
        // X axis (Red)
        0.0f, 0.0f, 0.0f,  X_AXIS_COLOR[0], X_AXIS_COLOR[1], X_AXIS_COLOR[2],
        axis_length_, 0.0f, 0.0f,  X_AXIS_COLOR[0], X_AXIS_COLOR[1], X_AXIS_COLOR[2],
        
        // Y axis (Green)
        0.0f, 0.0f, 0.0f,  Y_AXIS_COLOR[0], Y_AXIS_COLOR[1], Y_AXIS_COLOR[2],
        0.0f, axis_length_, 0.0f,  Y_AXIS_COLOR[0], Y_AXIS_COLOR[1], Y_AXIS_COLOR[2],
        
        // Z axis (Blue)
        0.0f, 0.0f, 0.0f,  Z_AXIS_COLOR[0], Z_AXIS_COLOR[1], Z_AXIS_COLOR[2],
        0.0f, 0.0f, axis_length_,  Z_AXIS_COLOR[0], Z_AXIS_COLOR[1], Z_AXIS_COLOR[2],
    };

    // Create vertex buffer
    vertex_buffer_ = gl_device_->create_buffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
    gl_device_->set_object_label(GL_BUFFER, vertex_buffer_.gl_id, "AxesVBO");

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

    gl_device_->set_object_label(GL_VERTEX_ARRAY, vao_, "AxesVAO");
}

void AxesPass::destroy_geometry() {
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
