#include "GraphicsContext.hpp"
#include "NullContext.hpp"

// Include backend headers based on compile-time configuration
#ifdef ASTRAEUS_ENABLE_EGL
#include "egl/EGLContext.hpp"
#endif

#ifdef WIN32
#include "wgl/WGLContext.hpp"
#endif

namespace astraeus {
    thread_local GraphicsContext* g_current_context = nullptr;

GraphicsContext* create_graphics_context() {
    // Platform-specific context creation based on compile-time configuration
    
#ifdef WIN32
    // Windows: use WGL backend
    return new WGLContext();
#elif defined(ASTRAEUS_ENABLE_EGL)
    // Linux/Unix: use EGL backend
    return new EGLGraphicsContext();
#else
    // Fallback: no backend available
    return new NullContext();
#endif
}

} // namespace astraeus
