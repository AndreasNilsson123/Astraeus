#include "ClearPass.hpp"
#include "../opengl/GLRenderDevice.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

namespace astraeus {

ClearPass::ClearPass()
    : gl_device_(nullptr)
{
    // Default clear color: dark blue
    clear_color_[0] = 0.1f;
    clear_color_[1] = 0.15f;
    clear_color_[2] = 0.2f;
    clear_color_[3] = 1.0f;
}

bool ClearPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }
    return true;
}

void ClearPass::execute(RenderDevice* device, World* world) {
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

void ClearPass::on_resize(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
    // Nothing to do for clear pass
}

} // namespace astraeus
