# Platform Abstraction Layer

## Overview

The `engine/platform/` module provides a minimal, stable interface for platform-specific operations in Astraeus. This layer isolates all platform-specific code to a single module, preventing scattered `#ifdef` statements throughout the codebase.

## Structure

```
engine/platform/
├── Platform.hpp           # Public interface (platform-agnostic)
├── Win32/
│   ├── Win32Headers.hpp   # Centralized Windows header includes
│   └── Win32Platform.cpp  # Windows implementation
├── Linux/
│   ├── X11Headers.hpp     # Centralized X11 header includes (stub)
│   └── LinuxPlatform.cpp  # Linux implementation
└── GL/
    └── GLHeaders.hpp      # Centralized OpenGL header includes
```

## Rules

### ✅ DO

1. **Use `Platform.hpp` for all platform operations**
   ```cpp
   #include "platform/Platform.hpp"
   
   uint64_t start = astraeus::platform::monotonic_time_ns();
   // ... work ...
   uint64_t elapsed = astraeus::platform::monotonic_time_ns() - start;
   ```

2. **Keep Platform.hpp interface minimal**
   - Only add functions when needed by multiple modules
   - All functions must work on Windows AND Linux

3. **Isolate platform headers to platform/ directory**
   - `<windows.h>` only in `platform/Win32/`
   - X11 headers only in `platform/Linux/`
   - GLAD headers centralized in `platform/GL/`

### ❌ DON'T

1. **Never include platform headers in public APIs**
   ```cpp
   // ❌ BAD: Leaks platform types
   #ifdef _WIN32
   #include <windows.h>
   HWND get_window_handle();
   #endif
   ```

2. **Never use scattered #ifdef statements**
   ```cpp
   // ❌ BAD: Scattered platform code
   void some_function() {
   #ifdef _WIN32
       // Windows code
   #else
       // Linux code
   #endif
   }
   ```

3. **Never call platform APIs directly**
   ```cpp
   // ❌ BAD: Direct Win32 call
   QueryPerformanceCounter(&counter);
   
   // ✅ GOOD: Use platform abstraction
   uint64_t time = astraeus::platform::monotonic_time_ns();
   ```

## API Reference

### Initialization

```cpp
void platform::init();
```

Initialize the platform module. Call once at engine startup.

**Example:**
```cpp
int main() {
    astraeus::platform::init();
    // ... rest of engine initialization ...
}
```

### Timing

```cpp
uint64_t platform::monotonic_time_ns();
```

Get monotonic time in nanoseconds. Never goes backwards, suitable for profiling.

**Example:**
```cpp
uint64_t start = astraeus::platform::monotonic_time_ns();
expensive_operation();
uint64_t elapsed = astraeus::platform::monotonic_time_ns() - start;
std::cout << "Operation took " << (elapsed / 1'000'000) << "ms\n";
```

### OpenGL Function Loading

```cpp
void* platform::load_gl_proc(const char* name);
```

Load an OpenGL function pointer. Platform-specific (wglGetProcAddress on Windows, glXGetProcAddress on Linux).

**Example:**
```cpp
// Typically used by GLAD or manual function loading
auto glCreateShader = (PFNGLCREATESHADERPROC)
    astraeus::platform::load_gl_proc("glCreateShader");
```

### Thread Debugging

```cpp
void platform::set_thread_name(const char* name);
```

Set the name of the current thread for debugging. Optional - may be no-op on some platforms. Thread name may be truncated to platform limits (e.g., 16 characters on Linux).

**Example:**
```cpp
// In a worker thread
astraeus::platform::set_thread_name("RenderThread");
```

### Memory Utilities

```cpp
size_t platform::get_page_size();
```

Get the system page size in bytes. Useful for memory alignment and allocation.

**Example:**
```cpp
size_t page_size = astraeus::platform::get_page_size();
void* aligned = aligned_alloc(page_size, buffer_size);
```

## Implementation Guide

### Adding a New Platform Function

1. **Check if it's truly needed**
   - Is it used by multiple modules?
   - Can it be implemented on all target platforms?

2. **Add declaration to Platform.hpp**
   ```cpp
   namespace astraeus::platform {
       // ... existing functions ...
       
       // New function
       bool new_feature();
   }
   ```

3. **Implement for Windows in Win32Platform.cpp**
   ```cpp
   #ifdef _WIN32
   namespace astraeus::platform {
       bool new_feature() {
           // Windows implementation using Win32 APIs
       }
   }
   #endif
   ```

4. **Implement for Linux in LinuxPlatform.cpp**
   ```cpp
   #ifdef __linux__
   namespace astraeus::platform {
       bool new_feature() {
           // Linux implementation using POSIX/X11 APIs
       }
   }
   #endif
   ```

5. **Document in this file and DEPENDENCIES.md**

### Example: Adding a File Dialog Function

```cpp
// Platform.hpp
namespace astraeus::platform {
    // Returns empty string if cancelled
    std::string open_file_dialog(const char* title, const char* filter);
}

// Win32Platform.cpp
#ifdef _WIN32
#include "Win32Headers.hpp"
#include <string>

namespace astraeus::platform {
    std::string open_file_dialog(const char* title, const char* filter) {
        OPENFILENAMEA ofn = {};
        char filename[MAX_PATH] = "";
        
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = title;
        ofn.lpstrFilter = filter;
        
        if (GetOpenFileNameA(&ofn)) {
            return std::string(filename);
        }
        return "";
    }
}
#endif

// LinuxPlatform.cpp
#ifdef __linux__
#include <string>

namespace astraeus::platform {
    std::string open_file_dialog(const char* title, const char* filter) {
        // Linux implementation using GTK or Zenity
        // For now, stub:
        return "";
    }
}
#endif
```

## Integration with Existing Code

The platform module is designed to coexist with existing platform-specific code (e.g., WGLContext, EGLContext). Over time, common platform operations should be migrated to use this abstraction.

### Current State

- Graphics backend (WGL/EGL) handles context creation and OpenGL setup
- Platform module provides utilities for timing, function loading, etc.
- Both can coexist - platform module doesn't replace graphics backend

### Future Migration

Consider migrating these operations to platform module:
- Window handle management (if needed outside graphics backend)
- High-resolution timer usage (consolidate with monotonic_time_ns)
- Thread utilities (if used outside engine core)

## Testing

### Include Hygiene Check

Run the automated check to verify platform isolation:

```bash
./.github/workflows/check-includes.sh
```

This script validates:
- No `<windows.h>` outside `engine/platform/Win32/`
- No platform `#ifdef` statements outside `engine/platform/`
- No X11 headers outside `engine/platform/Linux/`
- GLAD includes are properly isolated

### Manual Testing

Test on each platform:

**Windows:**
```bash
cd engine/build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

**Linux:**
```bash
cd engine/build
cmake ..
make -j
```

Verify:
1. Platform module compiles without errors
2. Timing functions return reasonable values
3. OpenGL function loading works with graphics backend

## See Also

- [DEPENDENCIES.md](../../docs/DEPENDENCIES.md) - Comprehensive dependency policy
- [ARCHITECTURE.md](../../docs/ARCHITECTURE.md) - Overall system architecture
- CMakeLists.txt - Build configuration for platform module
