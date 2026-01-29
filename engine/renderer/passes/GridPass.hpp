#ifndef ASTRAEUS_GRID_PASS_HPP
#define ASTRAEUS_GRID_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include "../../scene/World.hpp"
#include <cmath>
#include <iostream>
#include <vector>

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include <glad/glad.h>

namespace astraeus {

/**
 * GridPass: Renders a world-space grid on the XZ plane.
 * The grid fades with distance and uses proper 3D projection.
 */
class GridPass : public RenderPass {
public:
    inline GridPass();
    inline ~GridPass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;
    inline const char* get_name() const override { return "Grid"; }

    // Configuration
    void set_grid_size(float size) { grid_size_ = size; }
    void set_grid_spacing(float spacing) { grid_spacing_ = spacing; }
    void set_grid_color(float r, float g, float b) { 
        grid_color_[0] = r; 
        grid_color_[1] = g; 
        grid_color_[2] = b; 
    }
    void set_fade_distances(float start, float end) {
        fade_start_ = start;
        fade_end_ = end;
    }

private:
    inline void create_geometry();
    inline void destroy_geometry();

    GLRenderDevice* gl_device_;
    GLRenderDevice::ShaderHandle shader_;
    GLRenderDevice::BufferHandle vertex_buffer_;
    uint32_t vao_;
    uint32_t vertex_count_;

    // Grid parameters
    float grid_size_;      // Total size of the grid
    float grid_spacing_;   // Spacing between grid lines
    float grid_color_[3];  // RGB color
    float fade_start_;     // Distance where fade begins
    float fade_end_;       // Distance where fade completes
    
    uint32_t viewport_width_;
    uint32_t viewport_height_;
};

// Vertex and fragment shaders for the grid
inline constexpr const char* grid_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uViewProjection;

out vec3 worldPos;

void main() {
    worldPos = aPos;
    gl_Position = uViewProjection * vec4(aPos, 1.0);
}
)";

inline constexpr const char* grid_fragment_shader = R"(
#version 330 core
in vec3 worldPos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint EntityID;

uniform vec3 uCameraPos;
uniform vec3 uGridColor;
uniform float uFadeStart;
uniform float uFadeEnd;

void main() {
    // Fade with distance
    float distanceToCamera = length(worldPos - uCameraPos);
    float fade = 1.0 - smoothstep(uFadeStart, uFadeEnd, distanceToCamera);
    
    // Apply fade with visibility
    float alpha = fade * 0.8;
    
    FragColor = vec4(uGridColor, alpha);
    EntityID = 0u; // Grid has no entity ID
}
)";

// Implementation

inline GridPass::GridPass()
    : gl_device_(nullptr)
    , vao_(0)
    , vertex_count_(0)
    , grid_size_(100.0f)
    , grid_spacing_(1.0f)
    , grid_color_{0.5f, 0.5f, 0.5f}
    , fade_start_(50.0f)
    , fade_end_(150.0f)
    , viewport_width_(1280)
    , viewport_height_(720)
{
}

inline GridPass::~GridPass() {
    destroy_geometry();
}

inline bool GridPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    // Compile shader
    std::string error_msg;
    shader_ = gl_device_->create_shader(grid_vertex_shader, grid_fragment_shader, error_msg);
    if (shader_.gl_program == 0) {
        std::cerr << "[GridPass] Failed to create shader: " << error_msg << std::endl;
        return false;
    }

    gl_device_->set_object_label(GL_PROGRAM, shader_.gl_program, "GridShader");

    // Create geometry
    create_geometry();

    std::cout << "[GridPass] Initialized successfully" << std::endl;
    return true;
}

inline void GridPass::execute(RenderDevice* device, World* world) {
    (void)device;

    if (!gl_device_ || shader_.gl_program == 0 || !world) {
        return;
    }

    gl_device_->push_debug_group("GridPass");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable blending for fade effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable face culling (we want to see grid from both sides)
    glDisable(GL_CULL_FACE);

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

    float cam_x, cam_y, cam_z;
    camera.get_position(cam_x, cam_y, cam_z);
    
    GLint cam_pos_loc = glGetUniformLocation(shader_.gl_program, "uCameraPos");
    if (cam_pos_loc != -1) {
        glUniform3f(cam_pos_loc, cam_x, cam_y, cam_z);
    }

    GLint color_loc = glGetUniformLocation(shader_.gl_program, "uGridColor");
    if (color_loc != -1) {
        glUniform3f(color_loc, grid_color_[0], grid_color_[1], grid_color_[2]);
    }

    GLint fade_start_loc = glGetUniformLocation(shader_.gl_program, "uFadeStart");
    if (fade_start_loc != -1) {
        glUniform1f(fade_start_loc, fade_start_);
    }

    GLint fade_end_loc = glGetUniformLocation(shader_.gl_program, "uFadeEnd");
    if (fade_end_loc != -1) {
        glUniform1f(fade_end_loc, fade_end_);
    }

    // Draw grid
    glBindVertexArray(vao_);
    gl_device_->draw_arrays(GL_LINES, 0, vertex_count_);
    glBindVertexArray(0);

    // Restore state
    glDisable(GL_BLEND);

    gl_device_->pop_debug_group();
}

inline void GridPass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

inline void GridPass::create_geometry() {
    std::vector<float> vertices;

    // Generate grid lines
    int num_lines = static_cast<int>(grid_size_ / grid_spacing_);
    float half_size = grid_size_ / 2.0f;

    // Lines parallel to X axis (running along X, varying in Z)
    for (int i = -num_lines; i <= num_lines; ++i) {
        float z = i * grid_spacing_;
        
        // Line from (-half_size, 0, z) to (half_size, 0, z)
        vertices.push_back(-half_size);
        vertices.push_back(0.0f);
        vertices.push_back(z);
        
        vertices.push_back(half_size);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    // Lines parallel to Z axis (running along Z, varying in X)
    for (int i = -num_lines; i <= num_lines; ++i) {
        float x = i * grid_spacing_;
        
        // Line from (x, 0, -half_size) to (x, 0, half_size)
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(-half_size);
        
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(half_size);
    }

    vertex_count_ = static_cast<uint32_t>(vertices.size() / 3);

    // Create vertex buffer
    vertex_buffer_ = gl_device_->create_buffer(
        vertices.data(),
        static_cast<uint32_t>(vertices.size() * sizeof(float)),
        GL_STATIC_DRAW
    );
    gl_device_->set_object_label(GL_BUFFER, vertex_buffer_.gl_id, "GridVBO");

    // Create VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.gl_id);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gl_device_->set_object_label(GL_VERTEX_ARRAY, vao_, "GridVAO");
}

inline void GridPass::destroy_geometry() {
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

#endif // ASTRAEUS_GRID_PASS_HPP
