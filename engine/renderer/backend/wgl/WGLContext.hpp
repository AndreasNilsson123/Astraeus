#ifndef ASTRAEUS_WGL_CONTEXT_HPP
#define ASTRAEUS_WGL_CONTEXT_HPP

#include "../GraphicsContext.hpp"

namespace astraeus {

/**
 * WGL-based graphics context for Windows headless rendering.
 * Only compiled when building on Windows.
 */
class WGLContext : public GraphicsContext {
public:
    WGLContext();
    ~WGLContext() override;

    bool initialize(uint32_t width, uint32_t height) override;
    void shutdown() override;
    bool make_current() override;
    void* get_proc_address(const char* name) override;
    const char* get_backend_name() const override { return "WGL (Windows)"; }

private:
    void* hdc_;         // HDC
    void* hglrc_;       // HGLRC
    void* hwnd_;        // HWND for offscreen window
    uint32_t width_;
    uint32_t height_;
};

} // namespace astraeus

#endif // ASTRAEUS_WGL_CONTEXT_HPP
