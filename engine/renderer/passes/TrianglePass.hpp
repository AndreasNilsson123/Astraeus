#ifndef ASTRAEUS_TRIANGLE_PASS_HPP
#define ASTRAEUS_TRIANGLE_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"

namespace astraeus {

/**
 * TrianglePass: Renders an animated colored triangle for testing.
 */
class TrianglePass : public RenderPass {
public:
    TrianglePass();
    ~TrianglePass() override;

    bool initialize(RenderDevice* device) override;
    void execute(RenderDevice* device, World* world) override;
    void on_resize(uint32_t width, uint32_t height) override;

private:
    void create_geometry();
    void destroy_geometry();

    GLRenderDevice* gl_device_;
    GLRenderDevice::ShaderHandle shader_;
    GLRenderDevice::BufferHandle vertex_buffer_;
    uint32_t vao_;
    float rotation_angle_;
};

} // namespace astraeus

#endif // ASTRAEUS_TRIANGLE_PASS_HPP
