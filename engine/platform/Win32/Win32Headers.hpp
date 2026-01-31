#ifndef ASTRAEUS_WIN32_HEADERS_HPP
#define ASTRAEUS_WIN32_HEADERS_HPP

/**
 * Centralized Windows header includes for Astraeus.
 * 
 * This file is the ONLY place where <windows.h> should be included.
 * It ensures proper configuration to avoid common issues:
 * - NOMINMAX: Prevents min/max macro definitions that conflict with std::min/max
 * - WIN32_LEAN_AND_MEAN: Reduces header size by excluding rarely-used APIs
 * 
 * USAGE:
 * - Only include this file in engine/platform/Win32/.cpp files
 * - Never include this file in public headers
 * - Never include <windows.h> directly
 */

#ifdef _WIN32

// Configure Windows headers before inclusion
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Include Windows headers
#include <windows.h>

// Optionally include WGL for OpenGL on Windows
// (only if needed in this translation unit)
// #include <glad/glad_wgl.h>

#endif // _WIN32

#endif // ASTRAEUS_WIN32_HEADERS_HPP
