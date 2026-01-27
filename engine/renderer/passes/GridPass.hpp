#ifndef ASTRAEUS_GRID_PASS_HPP
#define ASTRAEUS_GRID_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"

namespace astraeus {

/**
 * GridPass: Renders a world-space grid on the XZ plane.
 * The grid fades with distance and uses proper 3D projection.
 */
class GridPass : public RenderPass {
public:
    GridPass();
    ~GridPass() override;

    bool initialize(RenderDevice* device) override;
    void execute(RenderDevice* device, World* world) override;
    void on_resize(uint32_t width, uint32_t height) override;

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
    void create_geometry();
    void destroy_geometry();

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

} // namespace astraeus

#endif // ASTRAEUS_GRID_PASS_HPP
