#ifndef ASTRAEUS_POINT_SPRITE_PASS_HPP
#define ASTRAEUS_POINT_SPRITE_PASS_HPP

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
 * PointSpritePass: Renders entities as point sprites.
 * Uses instancing for efficient batch rendering.
 */
class PointSpritePass : public RenderPass {
public:
    inline PointSpritePass();
    inline ~PointSpritePass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;
    inline const char* get_name() const override { return "PointSprite"; }

    // Configuration
    void set_point_size(float size) { point_size_ = size; }

private:
    inline void create_geometry();
    inline void destroy_geometry();
    inline void update_instance_data(World* world);

    GLRenderDevice* gl_device_;
    GLRenderDevice::ShaderHandle shader_;
    GLRenderDevice::BufferHandle vertex_buffer_;
    GLRenderDevice::BufferHandle instance_buffer_;
    uint32_t vao_;
    
    float point_size_;
    uint32_t viewport_width_;
    uint32_t viewport_height_;
    
    // Instance data buffers (CPU side)
    std::vector<float> instance_positions_;  // x,y,z per instance
    std::vector<float> instance_colors_;     // r,g,b,a per instance
    uint32_t instance_count_;
};

// Vertex shader for point sprites with instancing
inline constexpr const char* point_sprite_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aInstancePos;
layout (location = 2) in vec4 aInstanceColor;

uniform mat4 uViewProjection;
uniform float uPointSize;

out vec4 vColor;

void main() {
    vColor = aInstanceColor;
    gl_Position = uViewProjection * vec4(aPos + aInstancePos, 1.0);
    gl_PointSize = uPointSize;
}
)";

// Fragment shader for point sprites
inline constexpr const char* point_sprite_fragment_shader = R"(
#version 330 core
in vec4 vColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint EntityID;

void main() {
    // Make circular point sprites
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) {
        discard;
    }
    
    // Smooth edges
    float alpha = vColor.a * (1.0 - smoothstep(0.4, 0.5, dist));
    FragColor = vec4(vColor.rgb, alpha);
    // Entity ID output requires additional instance data and picking buffer integration
    EntityID = 0u;
}
)";

// Implementation

inline PointSpritePass::PointSpritePass()
    : gl_device_(nullptr)
    , vao_(0)
    , point_size_(10.0f)
    , viewport_width_(1280)
    , viewport_height_(720)
    , instance_count_(0)
{
}

inline PointSpritePass::~PointSpritePass() {
    destroy_geometry();
}

inline bool PointSpritePass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }

    // Compile shader
    std::string error_msg;
    shader_ = gl_device_->create_shader(point_sprite_vertex_shader, point_sprite_fragment_shader, error_msg);
    if (shader_.gl_program == 0) {
        std::cerr << "[PointSpritePass] Failed to create shader: " << error_msg << std::endl;
        return false;
    }

    gl_device_->set_object_label(GL_PROGRAM, shader_.gl_program, "PointSpriteShader");

    // Create geometry
    create_geometry();

    std::cout << "[PointSpritePass] Initialized successfully" << std::endl;
    return true;
}

inline void PointSpritePass::create_geometry() {
    // Create VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Create vertex buffer for point geometry (just origin)
    float vertices[] = { 0.0f, 0.0f, 0.0f };
    vertex_buffer_ = gl_device_->create_buffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
    
    gl_device_->set_object_label(GL_BUFFER, vertex_buffer_.gl_id, "PointSpriteVertexBuffer");
    
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.gl_id);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Create instance buffer (will be updated each frame)
    instance_buffer_ = gl_device_->create_buffer(nullptr, 0, GL_DYNAMIC_DRAW);
    gl_device_->set_object_label(GL_BUFFER, instance_buffer_.gl_id, "PointSpriteInstanceBuffer");

    glBindVertexArray(0);

    gl_device_->set_object_label(GL_VERTEX_ARRAY, vao_, "PointSpriteVAO");
}

inline void PointSpritePass::destroy_geometry() {
    if (gl_device_) {
        if (vao_ != 0) {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        if (vertex_buffer_.gl_id != 0) {
            gl_device_->destroy_buffer(vertex_buffer_);
            vertex_buffer_ = {};
        }
        if (instance_buffer_.gl_id != 0) {
            gl_device_->destroy_buffer(instance_buffer_);
            instance_buffer_ = {};
        }
    }
}

inline void PointSpritePass::update_instance_data(World* world) {
    if (!world) {
        instance_count_ = 0;
        return;
    }

    // Get all renderable entities
    const auto& entities = world->get_renderable_entities();
    
    instance_positions_.clear();
    instance_colors_.clear();
    instance_count_ = 0;

    for (uint32_t entity_id : entities) {
        const Renderable* renderable = world->get_entity_renderable(entity_id);
        if (!renderable || !renderable->visible) {
            continue;
        }

        const Transform* transform = world->get_entity_transform(entity_id);
        if (!transform) {
            continue;
        }

        const Color* color = world->get_entity_color(entity_id);
        
        // Add position
        instance_positions_.push_back(transform->pos_x);
        instance_positions_.push_back(transform->pos_y);
        instance_positions_.push_back(transform->pos_z);
        
        // Add color (use white if no color component)
        if (color) {
            instance_colors_.push_back(color->r);
            instance_colors_.push_back(color->g);
            instance_colors_.push_back(color->b);
            instance_colors_.push_back(color->a);
        } else {
            instance_colors_.push_back(1.0f);
            instance_colors_.push_back(1.0f);
            instance_colors_.push_back(1.0f);
            instance_colors_.push_back(1.0f);
        }
        
        instance_count_++;
    }
}

inline void PointSpritePass::execute(RenderDevice* device, World* world) {
    (void)device;

    if (!gl_device_ || shader_.gl_program == 0 || !world) {
        return;
    }

    // Update instance data
    update_instance_data(world);
    
    if (instance_count_ == 0) {
        return; // Nothing to render
    }

    // Upload instance data to GPU
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, instance_buffer_.gl_id);
    
    size_t total_size = (instance_positions_.size() + instance_colors_.size()) * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, total_size, nullptr, GL_DYNAMIC_DRAW);
    
    // Upload positions
    size_t pos_size = instance_positions_.size() * sizeof(float);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pos_size, instance_positions_.data());
    
    // Upload colors
    size_t color_size = instance_colors_.size() * sizeof(float);
    glBufferSubData(GL_ARRAY_BUFFER, pos_size, color_size, instance_colors_.data());
    
    // Set up instanced attributes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)pos_size);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // Enable point sprites
    glEnable(GL_PROGRAM_POINT_SIZE);
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
    
    GLint loc_point_size = glGetUniformLocation(shader_.gl_program, "uPointSize");
    if (loc_point_size >= 0) {
        glUniform1f(loc_point_size, point_size_);
    }

    // Draw instanced points
    glDrawArraysInstanced(GL_POINTS, 0, 1, instance_count_);

    // Cleanup
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glUseProgram(0);
}

inline void PointSpritePass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

} // namespace astraeus

#endif // ASTRAEUS_POINT_SPRITE_PASS_HPP
