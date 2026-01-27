#ifndef ASTRAEUS_EGL_CONTEXT_HPP
#define ASTRAEUS_EGL_CONTEXT_HPP

#include "../GraphicsContext.hpp"

namespace astraeus {

/**
 * EGL-based graphics context for Linux/Wayland headless rendering.
 * Only compiled when ASTRAEUS_ENABLE_EGL is ON.
 */
class EGLContext : public GraphicsContext {
public:
    EGLContext();
    ~EGLContext() override;

    bool initialize(uint32_t width, uint32_t height) override;
    void shutdown() override;
    bool make_current() override;
    void* get_proc_address(const char* name) override;
    const char* get_backend_name() const override { return "EGL (Linux)"; }

private:
    void* display_;
    void* context_;
    void* surface_;
    uint32_t width_;
    uint32_t height_;
};

} // namespace astraeus

#endif // ASTRAEUS_EGL_CONTEXT_HPP
