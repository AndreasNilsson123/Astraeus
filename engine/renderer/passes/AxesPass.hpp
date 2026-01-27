#ifndef ASTRAEUS_AXES_PASS_HPP
#define ASTRAEUS_AXES_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"

namespace astraeus {

/**
 * AxesPass: Renders XYZ coordinate axes at the origin.
 * X = Red, Y = Green, Z = Blue (standard convention).
 */
class AxesPass : public RenderPass {
public:
    AxesPass();
    ~AxesPass() override;

    bool initialize(RenderDevice* device) override;
    void execute(RenderDevice* device, World* world) override;
    void on_resize(uint32_t width, uint32_t height) override;

    // Configuration
    void set_axis_length(float length) { axis_length_ = length; }
    void set_line_width(float width) { line_width_ = width; }

private:
    void create_geometry();
    void destroy_geometry();

    GLRenderDevice* gl_device_;
    GLRenderDevice::ShaderHandle shader_;
    GLRenderDevice::BufferHandle vertex_buffer_;
    uint32_t vao_;
    
    float axis_length_;
    float line_width_;
    
    uint32_t viewport_width_;
    uint32_t viewport_height_;
};

} // namespace astraeus

#endif // ASTRAEUS_AXES_PASS_HPP
