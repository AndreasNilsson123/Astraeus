#ifndef ASTRAEUS_X11_HEADERS_HPP
#define ASTRAEUS_X11_HEADERS_HPP

/**
 * Centralized X11 header includes for Astraeus (Linux).
 * 
 * This file is the ONLY place where X11 headers should be included.
 * Similar to Win32Headers.hpp, this isolates platform-specific includes.
 * 
 * USAGE:
 * - Only include this file in engine/platform/Linux/*.cpp files
 * - Never include this file in public headers
 * - Never include X11 headers directly
 * 
 * NOTE: X11 support is currently stubbed for future implementation.
 */

#ifdef __linux__

// X11 headers would go here when implemented
// #include <X11/Xlib.h>
// #include <X11/Xutil.h>

#endif // __linux__

#endif // ASTRAEUS_X11_HEADERS_HPP
