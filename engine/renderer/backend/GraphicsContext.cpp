#include "GraphicsContext.hpp"
#include "NullContext.hpp"

// Include backend headers based on compile-time configuration
#ifdef ASTRAEUS_ENABLE_EGL
#include "egl/EGLContext.hpp"
#endif

#ifdef ASTRAEUS_ENABLE_WGL
#include "wgl/WGLContext.hpp"
#endif

namespace astraeus {

GraphicsContext* create_graphics_context() {
    // Platform-specific context creation based on compile-time configuration
    
#ifdef ASTRAEUS_ENABLE_WGL
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
