#ifndef ASTRAEUS_CLEAR_PASS_HPP
#define ASTRAEUS_CLEAR_PASS_HPP

#include "../RenderGraph.hpp"

namespace astraeus {

class GLRenderDevice;

/**
 * ClearPass: Clears the color and ID buffers at the start of each frame.
 */
class ClearPass : public RenderPass {
public:
    ClearPass();
    ~ClearPass() override = default;

    bool initialize(RenderDevice* device) override;
    void execute(RenderDevice* device, World* world) override;
    void on_resize(uint32_t width, uint32_t height) override;

private:
    GLRenderDevice* gl_device_;
    float clear_color_[4];
};

} // namespace astraeus

#endif // ASTRAEUS_CLEAR_PASS_HPP
