#include "../Platform.hpp"

#ifdef _WIN32

#include "Win32Headers.hpp"
#include <timeapi.h>  // For timeBeginPeriod (high-resolution timing)
#include <vector>

namespace astraeus::platform {

void init() {
    // Initialize high-resolution timing on Windows
    // Requests 1ms timer resolution (improves Sleep() accuracy)
    timeBeginPeriod(1);
}

uint64_t monotonic_time_ns() {
    // Use QueryPerformanceCounter for high-resolution monotonic time
    LARGE_INTEGER frequency, counter;
    
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    
    // Convert to nanoseconds
    // counter.QuadPart is in ticks, frequency.QuadPart is ticks per second
    return (counter.QuadPart * 1'000'000'000ULL) / frequency.QuadPart;
}

void* load_gl_proc(const char* name) {
    // Use wglGetProcAddress for OpenGL function loading on Windows
    // Note: This requires an active OpenGL context
    void* proc = reinterpret_cast<void*>(wglGetProcAddress(name));
    
    // wglGetProcAddress only works for extension functions
    // For core OpenGL 1.1 functions, need to use GetProcAddress on opengl32.dll
    if (proc == nullptr) {
        // C++17 guarantees thread-safe static local initialization
        static HMODULE opengl32 = LoadLibraryA("opengl32.dll");
        if (opengl32) {
            proc = reinterpret_cast<void*>(GetProcAddress(opengl32, name));
        }
    }
    
    return proc;
}

void set_thread_name(const char* name) {
    // Use SetThreadDescription (Windows 10+) for thread naming
    // Convert to wide string
    int len = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
    if (len > 0) {
        std::vector<wchar_t> wide_name(len);
        MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_name.data(), len);
        
        HANDLE thread = GetCurrentThread();
        SetThreadDescription(thread, wide_name.data());
    }
}

size_t get_page_size() {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return static_cast<size_t>(system_info.dwPageSize);
}

} // namespace astraeus::platform

#endif // _WIN32
