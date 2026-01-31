#include "../Platform.hpp"

#ifdef __linux__

#include "X11Headers.hpp"
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>

namespace astraeus::platform {

void init() {
    // Linux initialization (currently no-op)
    // Future: Initialize X11 or Wayland display connection if needed
}

uint64_t monotonic_time_ns() {
    // Use clock_gettime with CLOCK_MONOTONIC for high-resolution time
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    // Convert to nanoseconds
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL + 
           static_cast<uint64_t>(ts.tv_nsec);
}

void* load_gl_proc(const char* name) {
    // Use glXGetProcAddress for OpenGL function loading on Linux
    // Note: This requires linking with GLX
    
    // For now, use dlsym as a fallback (works for core functions)
    static void* libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!libgl) {
        libgl = dlopen("libGL.so", RTLD_LAZY | RTLD_LOCAL);
    }
    
    if (libgl) {
        return dlsym(libgl, name);
    }
    
    return nullptr;
}

void set_thread_name(const char* name) {
    // Use pthread_setname_np for thread naming on Linux
    // Thread name is limited to 16 characters including null terminator
    pthread_setname_np(pthread_self(), name);
}

size_t get_page_size() {
    // Use sysconf to get page size on Linux
    long page_size = sysconf(_SC_PAGESIZE);
    return (page_size > 0) ? static_cast<size_t>(page_size) : 4096;
}

} // namespace astraeus::platform

#endif // __linux__
