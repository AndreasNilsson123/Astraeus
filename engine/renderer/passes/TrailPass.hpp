#ifndef ASTRAEUS_TRAIL_PASS_HPP
#define ASTRAEUS_TRAIL_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include "../../scene/World.hpp"
#include <cmath>
#include <iostream>
#include <vector>

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

/**
 * TrailPass: Renders entity trails as polylines.
 * Trails are updated smoothly without per-frame reallocations.
 */
class TrailPass : public RenderPass {
public:
    inline TrailPass();
    inline ~TrailPass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;
    inline const char* get_name() const override { return "Trail"; }

    // Configuration
    void set_trail_width(float width) { trail_width_ = width; }
    void set_fade_alpha(bool fade) { fade_alpha_ = fade; }

private:
    inline void create_geometry();
    inline void destroy_geometry();
    inline void update_trail_data(World* world);

    GLRenderDevice* gl_device_;
    GLRenderDevice::ShaderHandle shader_;
    GLRenderDevice::BufferHandle vertex_buffer_;
    uint32_t vao_;
    
    float trail_width_;
    bool fade_alpha_;
    uint32_t viewport_width_;
    uint32_t viewport_height_;
    
    // Trail vertex data (CPU side)
    std::vector<float> trail_vertices_;  // x,y,z per vertex
    std::vector<float> trail_colors_;    // r,g,b,a per vertex
    uint32_t vertex_count_;
};

// Vertex shader for trails
inline constexpr const char* trail_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 uViewProjection;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = uViewProjection * vec4(aPos, 1.0);
}
)";

// Fragment shader for trails
inline constexpr const char* trail_fragment_shader = R"(
#version 330 core
in vec4 vColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint EntityID;

void main() {
    FragColor = vColor;
    EntityID = 0u; // Trails have no entity ID
}
)";

// Implementation

inline TrailPass::TrailPass()
    : gl_device_(nullptr)
    , vao_(0)
    , trail_width_(2.0f)
    , fade_alpha_(true)
    , viewport_width_(1280)
    , viewport_height_(720)
    , vertex_count_(0)
{
}

inline TrailPass::~TrailPass() {
    destroy_geometry();
}

inline bool TrailPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    // Compile shader
    std::string error_msg;
    shader_ = gl_device_->create_shader(trail_vertex_shader, trail_fragment_shader, error_msg);
    if (shader_.gl_program == 0) {
        std::cerr << "[TrailPass] Failed to create shader: " << error_msg << std::endl;
        return false;
    }

    gl_device_->set_object_label(GL_PROGRAM, shader_.gl_program, "TrailShader");

    // Create geometry
    create_geometry();

    std::cout << "[TrailPass] Initialized successfully" << std::endl;
    return true;
}

inline void TrailPass::create_geometry() {
    // Create VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Create vertex buffer (will be updated each frame)
    vertex_buffer_ = gl_device_->create_buffer(nullptr, 0, GL_DYNAMIC_DRAW);
    gl_device_->set_object_label(GL_BUFFER, vertex_buffer_.gl_id, "TrailVertexBuffer");

    glBindVertexArray(0);
    gl_device_->set_object_label(GL_VERTEX_ARRAY, vao_, "TrailVAO");
}

inline void TrailPass::destroy_geometry() {
    if (gl_device_) {
        if (vao_ != 0) {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        if (vertex_buffer_.gl_id != 0) {
            gl_device_->destroy_buffer(vertex_buffer_);
            vertex_buffer_ = {};
        }
    }
}

inline void TrailPass::update_trail_data(World* world) {
    if (!world) {
        vertex_count_ = 0;
        return;
    }

    trail_vertices_.clear();
    trail_colors_.clear();
    vertex_count_ = 0;

    // Get all renderable entities
    const auto& entities = world->get_renderable_entities();
    
    for (uint32_t entity_id : entities) {
        const TrackTrail* trail = world->get_entity_trail(entity_id);
        if (!trail || trail->current_count < 2) {
            continue; // Need at least 2 points for a trail
        }

        const Color* color = world->get_entity_color(entity_id);
        float base_r = color ? color->r : 1.0f;
        float base_g = color ? color->g : 1.0f;
        float base_b = color ? color->b : 1.0f;
        float base_a = color ? color->a : 1.0f;

        // Extract trail points from circular buffer
        uint32_t oldest_idx = (trail->head_index + 1) % trail->max_points;
        if (trail->current_count < trail->max_points) {
            oldest_idx = 0; // Buffer not full yet, start from beginning
        }

        for (uint32_t i = 0; i < trail->current_count; ++i) {
            uint32_t idx = (oldest_idx + i) % trail->max_points;
            uint32_t base_pos = idx * 3;

            // Add vertex position
            trail_vertices_.push_back(trail->positions[base_pos + 0]);
            trail_vertices_.push_back(trail->positions[base_pos + 1]);
            trail_vertices_.push_back(trail->positions[base_pos + 2]);

            // Calculate alpha fade (oldest = transparent, newest = opaque)
            // Avoid division by zero: if current_count is 1, alpha_fade is 1.0
            float alpha_fade = 1.0f;
            if (fade_alpha_ && trail->current_count > 1) {
                alpha_fade = static_cast<float>(i) / static_cast<float>(trail->current_count - 1);
            }

            // Add vertex color
            trail_colors_.push_back(base_r);
            trail_colors_.push_back(base_g);
            trail_colors_.push_back(base_b);
            trail_colors_.push_back(base_a * alpha_fade);

            vertex_count_++;
        }
    }
}

inline void TrailPass::execute(RenderDevice* device, World* world) {
    (void)device;

    if (!gl_device_ || shader_.gl_program == 0 || !world) {
        return;
    }

    // Update trail data
    update_trail_data(world);
    
    if (vertex_count_ == 0) {
        return; // Nothing to render
    }

    // Upload trail data to GPU
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.gl_id);
    
    size_t total_size = (trail_vertices_.size() + trail_colors_.size()) * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, total_size, nullptr, GL_DYNAMIC_DRAW);
    
    // Upload positions
    size_t pos_size = trail_vertices_.size() * sizeof(float);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pos_size, trail_vertices_.data());
    
    // Upload colors
    size_t color_size = trail_colors_.size() * sizeof(float);
    glBufferSubData(GL_ARRAY_BUFFER, pos_size, color_size, trail_colors_.data());
    
    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)pos_size);
    glEnableVertexAttribArray(1);

    // Enable line rendering
    glLineWidth(trail_width_);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader
    glUseProgram(shader_.gl_program);

    // Set uniforms
    const Camera& camera = world->get_camera();
    const float* vp_matrix = camera.get_view_projection_matrix();
    
    GLint loc_vp = glGetUniformLocation(shader_.gl_program, "uViewProjection");
    if (loc_vp >= 0) {
        glUniformMatrix4fv(loc_vp, 1, GL_FALSE, vp_matrix);
    }

    // Draw trails as line strips
    // For simplicity, we draw all vertices as one continuous line strip
    // In production, you'd want to track segments per entity
    uint32_t offset = 0;
    const auto& entities = world->get_renderable_entities();
    
    for (uint32_t entity_id : entities) {
        const TrackTrail* trail = world->get_entity_trail(entity_id);
        if (!trail || trail->current_count < 2) {
            continue;
        }
        
        glDrawArrays(GL_LINE_STRIP, offset, trail->current_count);
        offset += trail->current_count;
    }

    // Cleanup
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glUseProgram(0);
}

inline void TrailPass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

} // namespace astraeus

#endif // ASTRAEUS_TRAIL_PASS_HPP
