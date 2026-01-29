#ifndef ASTRAEUS_CLEAR_PASS_HPP
#define ASTRAEUS_CLEAR_PASS_HPP

#include "../RenderGraph.hpp"
#include "../opengl/GLRenderDevice.hpp"

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include <glad/glad.h>

namespace astraeus {

/**
 * ClearPass: Clears the color and ID buffers at the start of each frame.
 */
class ClearPass : public RenderPass {
public:
    inline ClearPass();
    ~ClearPass() override = default;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;

private:
    GLRenderDevice* gl_device_;
    float clear_color_[4];
};

// Implementation

inline ClearPass::ClearPass()
    : gl_device_(nullptr)
{
    // Default clear color: dark blue
    clear_color_[0] = 0.1f;
    clear_color_[1] = 0.15f;
    clear_color_[2] = 0.2f;
    clear_color_[3] = 1.0f;
}

inline bool ClearPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }
    return true;
}

inline void ClearPass::execute(RenderDevice* device, World* world) {
    (void)device;
    (void)world;

    if (!gl_device_) {
        return;
    }

    gl_device_->push_debug_group("ClearPass");

    // Clear color buffer
    glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Clear ID buffer to 0 (no entity)
    GLuint clear_value = 0;
    glClearBufferuiv(GL_COLOR, 1, &clear_value);

    gl_device_->pop_debug_group();
}

inline void ClearPass::on_resize(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
    // Nothing to do for clear pass
}

} // namespace astraeus

#endif // ASTRAEUS_CLEAR_PASS_HPP
