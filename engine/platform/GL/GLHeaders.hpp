#ifndef ASTRAEUS_GL_HEADERS_HPP
#define ASTRAEUS_GL_HEADERS_HPP

/**
 * Centralized OpenGL header includes for Astraeus.
 * 
 * This file isolates GLAD and OpenGL headers to prevent them from
 * polluting the rest of the codebase. Only renderer backend code
 * should include this file.
 * 
 * RULES:
 * - Only include this in renderer backend implementation files
 * - Never include this in public headers
 * - Never include GLAD or OpenGL headers directly
 * 
 * USAGE:
 * - Include this file in renderer backend .cpp files that need OpenGL
 * - Use the platform abstraction (platform::load_gl_proc) for function loading
 */

// Include GLAD for OpenGL function loading
// GLAD must be included before any other OpenGL headers
#include "platform/GL/GLHeaders.hpp"

// Platform-specific GL extensions (if needed)
#ifdef _WIN32
    // WGL extensions loaded via GLAD
    #include <glad/glad_wgl.h>
#elif defined(__linux__)
    // GLX extensions (would be loaded via GLAD or directly)
    // #include <glad/glad_glx.h>
#endif

#endif // ASTRAEUS_GL_HEADERS_HPP
