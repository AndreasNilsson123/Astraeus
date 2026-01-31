#ifndef ASTRAEUS_PLATFORM_HPP
#define ASTRAEUS_PLATFORM_HPP

#include <cstdint>
#include <cstddef>

/**
 * Platform abstraction layer for Astraeus.
 * 
 * This module provides a minimal, stable interface for platform-specific operations.
 * All platform-specific code (Win32, X11, etc.) is isolated to engine/platform/ and
 * accessed only through this interface.
 * 
 * RULES:
 * - Only engine/platform/ may contain #ifdef _WIN32, #ifdef __linux__, etc.
 * - Only engine/platform/ may include <windows.h>, X11 headers, etc.
 * - All other engine code must use this interface for platform operations.
 * - Keep this interface minimal - add functions only when needed by multiple modules.
 */
namespace astraeus::platform {

/**
 * Initialize the platform module.
 * Call once at engine startup before any other platform functions.
 */
void init();

/**
 * Get monotonic time in nanoseconds.
 * Guaranteed to never go backwards, suitable for timing and profiling.
 * 
 * @return Time in nanoseconds since an arbitrary epoch
 */
uint64_t monotonic_time_ns();

/**
 * Load an OpenGL function pointer.
 * Platform-specific (wglGetProcAddress on Windows, glXGetProcAddress on Linux, etc.)
 * 
 * @param name Function name to look up (e.g., "glCreateShader")
 * @return Function pointer or nullptr if not found
 */
void* load_gl_proc(const char* name);

/**
 * Set the name of the current thread (for debugging).
 * Optional - may be no-op on some platforms.
 * 
 * @param name Thread name (will be truncated to platform limits)
 */
void set_thread_name(const char* name);

/**
 * Get the system page size in bytes.
 * Useful for memory alignment and allocation.
 * 
 * @return Page size in bytes (typically 4096)
 */
size_t get_page_size();

} // namespace astraeus::platform

#endif // ASTRAEUS_PLATFORM_HPP
