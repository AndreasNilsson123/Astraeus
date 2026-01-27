#include "EGLContext.hpp"
#include <iostream>
#include <cstring>

// EGL headers - only included in this backend implementation
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace astraeus {

EGLContext::EGLContext()
    : display_(nullptr)
    , context_(nullptr)
    , surface_(nullptr)
    , width_(0)
    , height_(0)
{
}

EGLContext::~EGLContext() {
    shutdown();
}

bool EGLContext::initialize(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;

    std::cout << "[EGLContext] Initializing EGL backend for " << width << "x" << height << std::endl;

    // Query EGL extensions to find the best display creation method
    const char* extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    bool has_platform_base = extensions && strstr(extensions, "EGL_EXT_platform_base");
    bool has_surfaceless = extensions && strstr(extensions, "EGL_MESA_platform_surfaceless");

    EGLDisplay display = EGL_NO_DISPLAY;
    
    // Try to use surfaceless platform (best for headless)
    if (has_platform_base && has_surfaceless) {
        auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
            eglGetProcAddress("eglGetPlatformDisplayEXT");
        if (eglGetPlatformDisplayEXT) {
            display = eglGetPlatformDisplayEXT(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
            std::cout << "[EGLContext] Using EGL_PLATFORM_SURFACELESS_MESA" << std::endl;
        }
    }
    
    // Fallback to default display
    if (display == EGL_NO_DISPLAY) {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        std::cout << "[EGLContext] Using EGL default display" << std::endl;
    }
    
    if (display == EGL_NO_DISPLAY) {
        std::cerr << "[EGLContext] Failed to get EGL display" << std::endl;
        return false;
    }
    
    display_ = display;

    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(static_cast<EGLDisplay>(display_), &major, &minor)) {
        std::cerr << "[EGLContext] Failed to initialize EGL: error " << eglGetError() << std::endl;
        return false;
    }

    std::cout << "[EGLContext] EGL version: " << major << "." << minor << std::endl;

    // Choose config
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(static_cast<EGLDisplay>(display_), config_attribs, &config, 1, &num_configs)) {
        std::cerr << "[EGLContext] Failed to choose EGL config" << std::endl;
        return false;
    }

    // Bind OpenGL API
    if (!eglBindAPI(EGL_OPENGL_API)) {
        std::cerr << "[EGLContext] Failed to bind OpenGL API" << std::endl;
        return false;
    }

    // Create context
    EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    EGLContext context = eglCreateContext(static_cast<EGLDisplay>(display_), config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "[EGLContext] Failed to create EGL context: error " << eglGetError() << std::endl;
        return false;
    }

    // Create pbuffer surface
    EGLint pbuffer_attribs[] = {
        EGL_WIDTH, static_cast<EGLint>(width_),
        EGL_HEIGHT, static_cast<EGLint>(height_),
        EGL_NONE
    };

    EGLSurface surface = eglCreatePbufferSurface(static_cast<EGLDisplay>(display_), config, pbuffer_attribs);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "[EGLContext] Failed to create pbuffer surface: error " << eglGetError() << std::endl;
        eglDestroyContext(static_cast<EGLDisplay>(display_), context);
        return false;
    }

    context_ = context;
    surface_ = surface;

    // Make context current
    if (!make_current()) {
        std::cerr << "[EGLContext] Failed to make context current" << std::endl;
        eglDestroySurface(static_cast<EGLDisplay>(display_), static_cast<EGLSurface>(surface_));
        eglDestroyContext(static_cast<EGLDisplay>(display_), static_cast<EGLContext>(context_));
        context_ = nullptr;
        surface_ = nullptr;
        return false;
    }

    std::cout << "[EGLContext] EGL context initialized successfully" << std::endl;
    return true;
}

void EGLContext::shutdown() {
    if (context_) {
        eglMakeCurrent(static_cast<EGLDisplay>(display_), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(static_cast<EGLDisplay>(display_), static_cast<EGLContext>(context_));
        context_ = nullptr;
    }

    if (surface_) {
        eglDestroySurface(static_cast<EGLDisplay>(display_), static_cast<EGLSurface>(surface_));
        surface_ = nullptr;
    }

    if (display_) {
        eglTerminate(static_cast<EGLDisplay>(display_));
        display_ = nullptr;
    }

    std::cout << "[EGLContext] Shutdown complete" << std::endl;
}

bool EGLContext::make_current() {
    if (!display_ || !surface_ || !context_) {
        return false;
    }

    return eglMakeCurrent(
        static_cast<EGLDisplay>(display_),
        static_cast<EGLSurface>(surface_),
        static_cast<EGLSurface>(surface_),
        static_cast<EGLContext>(context_)
    );
}

void* EGLContext::get_proc_address(const char* name) {
    return reinterpret_cast<void*>(eglGetProcAddress(name));
}

} // namespace astraeus
