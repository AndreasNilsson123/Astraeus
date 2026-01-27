#ifndef ASTRAEUS_POINT_SPRITE_PASS_HPP
#define ASTRAEUS_POINT_SPRITE_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"

namespace astraeus {

/**
 * PointSpritePass: Renders entities as point sprites.
 * Uses instancing for efficient batch rendering.
 */
class PointSpritePass : public RenderPass {
public:
    PointSpritePass();
    ~PointSpritePass() override;

    bool initialize(RenderDevice* device) override;
    void execute(RenderDevice* device, World* world) override;
    void on_resize(uint32_t width, uint32_t height) override;

    // Configuration
    void set_point_size(float size) { point_size_ = size; }

private:
    void create_geometry();
    void destroy_geometry();
    void update_instance_data(World* world);

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

} // namespace astraeus

#endif // ASTRAEUS_POINT_SPRITE_PASS_HPP
