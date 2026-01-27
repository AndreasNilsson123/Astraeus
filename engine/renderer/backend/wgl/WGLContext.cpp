#include "WGLContext.hpp"
#include <iostream>

// Windows headers - only included in this backend implementation
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/wglext.h>
#endif

namespace astraeus {

WGLContext::WGLContext()
    : hdc_(nullptr)
    , hglrc_(nullptr)
    , hwnd_(nullptr)
    , width_(0)
    , height_(0)
{
}

WGLContext::~WGLContext() {
    shutdown();
}

bool WGLContext::initialize(uint32_t width, uint32_t height) {
#ifdef _WIN32
    width_ = width;
    height_ = height;

    std::cout << "[WGLContext] Initializing WGL backend for " << width << "x" << height << std::endl;

    // Create a dummy window for the device context
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = hInstance;
    wc.lpszClassName = "AstraeusOffscreenGL";
    wc.style = CS_OWNDC;
    
    if (!RegisterClassA(&wc)) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            std::cerr << "[WGLContext] Failed to register window class: " << error << std::endl;
            return false;
        }
    }

    HWND hwnd = CreateWindowExA(
        0,
        "AstraeusOffscreenGL",
        "Offscreen",
        WS_OVERLAPPEDWINDOW,
        0, 0, static_cast<int>(width_), static_cast<int>(height_),
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        std::cerr << "[WGLContext] Failed to create window: " << GetLastError() << std::endl;
        return false;
    }

    hwnd_ = hwnd;

    // Get device context
    HDC hdc = GetDC(static_cast<HWND>(hwnd_));
    if (!hdc) {
        std::cerr << "[WGLContext] Failed to get device context" << std::endl;
        return false;
    }

    hdc_ = hdc;

    // Set pixel format
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(static_cast<HDC>(hdc_), &pfd);
    if (pixelFormat == 0) {
        std::cerr << "[WGLContext] Failed to choose pixel format: " << GetLastError() << std::endl;
        return false;
    }

    if (!SetPixelFormat(static_cast<HDC>(hdc_), pixelFormat, &pfd)) {
        std::cerr << "[WGLContext] Failed to set pixel format: " << GetLastError() << std::endl;
        return false;
    }

    // Create OpenGL context
    HGLRC hglrc = wglCreateContext(static_cast<HDC>(hdc_));
    if (!hglrc) {
        std::cerr << "[WGLContext] Failed to create OpenGL context: " << GetLastError() << std::endl;
        return false;
    }

    hglrc_ = hglrc;

    // Make context current
    if (!make_current()) {
        std::cerr << "[WGLContext] Failed to make context current" << std::endl;
        return false;
    }

    // Try to create a modern OpenGL 3.3 context using wglCreateContextAttribsARB
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
        reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
            wglGetProcAddress("wglCreateContextAttribsARB")
        );

    if (wglCreateContextAttribsARB) {
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        HGLRC modern_context = wglCreateContextAttribsARB(static_cast<HDC>(hdc_), nullptr, attribs);
        if (modern_context) {
            // Switch to modern context
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(static_cast<HGLRC>(hglrc_));
            hglrc_ = modern_context;
            
            if (!make_current()) {
                std::cerr << "[WGLContext] Failed to make modern context current" << std::endl;
                return false;
            }
            
            std::cout << "[WGLContext] Created OpenGL 3.3 core profile context" << std::endl;
        } else {
            std::cout << "[WGLContext] Using legacy OpenGL context" << std::endl;
        }
    } else {
        std::cout << "[WGLContext] wglCreateContextAttribsARB not available, using legacy context" << std::endl;
    }

    std::cout << "[WGLContext] WGL context initialized successfully" << std::endl;
    return true;
#else
    std::cerr << "[WGLContext] WGL backend only available on Windows" << std::endl;
    return false;
#endif
}

void WGLContext::shutdown() {
#ifdef _WIN32
    if (hglrc_) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(static_cast<HGLRC>(hglrc_));
        hglrc_ = nullptr;
    }

    if (hdc_) {
        ReleaseDC(static_cast<HWND>(hwnd_), static_cast<HDC>(hdc_));
        hdc_ = nullptr;
    }

    if (hwnd_) {
        DestroyWindow(static_cast<HWND>(hwnd_));
        hwnd_ = nullptr;
    }

    std::cout << "[WGLContext] Shutdown complete" << std::endl;
#endif
}

bool WGLContext::make_current() {
#ifdef _WIN32
    if (!hdc_ || !hglrc_) {
        return false;
    }

    return wglMakeCurrent(static_cast<HDC>(hdc_), static_cast<HGLRC>(hglrc_)) == TRUE;
#else
    return false;
#endif
}

void* WGLContext::get_proc_address(const char* name) {
#ifdef _WIN32
    void* proc = reinterpret_cast<void*>(wglGetProcAddress(name));
    if (!proc) {
        // Try getting from opengl32.dll for core functions
        static HMODULE opengl32 = LoadLibraryA("opengl32.dll");
        if (opengl32) {
            proc = reinterpret_cast<void*>(GetProcAddress(opengl32, name));
        }
    }
    return proc;
#else
    return nullptr;
#endif
}

} // namespace astraeus
