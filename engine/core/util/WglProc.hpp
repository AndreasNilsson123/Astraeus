#ifndef ASTRAEUS_WGL_PROC_HPP
#define ASTRAEUS_WGL_PROC_HPP

/**
 * Typed WGL function pointer loader.
 * 
 * Centralizes the "funky cast" pattern needed for wglGetProcAddress
 * to eliminate scattered cast-function-type warnings.
 * 
 * Usage:
 *   auto wglCreateContextAttribsARB = 
 *       load_wgl_proc<PFNWGLCREATECONTEXTATTRIBSARBPROC>("wglCreateContextAttribsARB");
 */

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>

namespace astraeus {
namespace util {

/**
 * Load a WGL extension function with proper typing.
 * 
 * @tparam Fn Function pointer type (e.g., PFNWGLCREATECONTEXTATTRIBSARBPROC)
 * @param name Function name (e.g., "wglCreateContextAttribsARB")
 * @return Function pointer or nullptr on failure
 */
template <typename Fn>
inline Fn load_wgl_proc(const char* name) {
    if (!name) {
        return nullptr;
    }
    
    // Get proc address from WGL
    PROC proc = wglGetProcAddress(name);
    if (!proc) {
        return nullptr;
    }
    
    // Perform the cast in one centralized location
    // This is the only place where we need to suppress the warning
#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    
    Fn func = reinterpret_cast<Fn>(proc);
    
#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
    
    return func;
}

} // namespace util
} // namespace astraeus

#endif // _WIN32

#endif // ASTRAEUS_WGL_PROC_HPP
