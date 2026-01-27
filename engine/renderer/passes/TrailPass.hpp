#ifndef ASTRAEUS_TRAIL_PASS_HPP
#define ASTRAEUS_TRAIL_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"
#include <vector>

namespace astraeus {

/**
 * TrailPass: Renders entity trails as polylines.
 * Trails are updated smoothly without per-frame reallocations.
 */
class TrailPass : public RenderPass {
public:
    TrailPass();
    ~TrailPass() override;

    bool initialize(RenderDevice* device) override;
    void execute(RenderDevice* device, World* world) override;
    void on_resize(uint32_t width, uint32_t height) override;

    // Configuration
    void set_trail_width(float width) { trail_width_ = width; }
    void set_fade_alpha(bool fade) { fade_alpha_ = fade; }

private:
    void create_geometry();
    void destroy_geometry();
    void update_trail_data(World* world);

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

} // namespace astraeus

#endif // ASTRAEUS_TRAIL_PASS_HPP
