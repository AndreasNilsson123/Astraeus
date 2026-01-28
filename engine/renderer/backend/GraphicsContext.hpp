#ifndef ASTRAEUS_GRAPHICS_CONTEXT_HPP
#define ASTRAEUS_GRAPHICS_CONTEXT_HPP

#include <cstdint>

namespace astraeus {

/**
 * Abstract interface for platform-specific graphics context creation.
 * Implementations provide EGL (Linux), WGL (Windows), or Null (fallback) backends.
 */
class GraphicsContext {
public:
    virtual ~GraphicsContext() = default;

    /**
     * Initialize the graphics context with the given dimensions.
     * @return true on success, false on failure
     */
    virtual bool initialize(uint32_t width, uint32_t height) = 0;

    /**
     * Shutdown and cleanup the graphics context.
     */
    virtual void shutdown() = 0;

    /**
     * Make this context current for rendering.
     * @return true on success, false on failure
     */
    virtual bool make_current() = 0;

    /**
     * Get a platform-specific function pointer.
     * @param name Function name to look up
     * @return Function pointer or nullptr if not found
     */
    virtual void* get_proc_address(const char* name) = 0;

    /**
     * Get backend type as string for debugging.
     */
    virtual const char* get_backend_name() const = 0;
};

inline thread_local GraphicsContext* g_current_context = nullptr;

} // namespace astraeus

// Include concrete implementations after the interface is defined
#include "NullContext.hpp"

#ifdef ASTRAEUS_ENABLE_EGL
#include "egl/EGLContext.hpp"
#endif

#ifdef WIN32
#include "wgl/WGLContext.hpp"
#endif

namespace astraeus {

/**
 * Factory function to create the appropriate context for the current platform.
 * Returns EGLContext on Linux, WGLContext on Windows, or NullContext as fallback.
 */
inline GraphicsContext* create_graphics_context() {
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

#endif // ASTRAEUS_GRAPHICS_CONTEXT_HPP
