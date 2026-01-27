# Graphics Backend Architecture

## Overview

Astraeus uses a platform-abstracted graphics context system to support multiple platforms without requiring platform-specific graphics libraries on all systems.

## Backend Types

### 1. EGL Backend (Linux/Unix)
- **File**: `engine/renderer/backend/egl/EGLContext.cpp`
- **Platform**: Linux, Unix, Wayland
- **Requirements**: Mesa EGL development libraries (`libegl1-mesa-dev`)
- **Use Case**: Headless rendering on Linux servers, workstations
- **Enabled By**: `ASTRAEUS_ENABLE_EGL=ON` (default on Linux)

### 2. WGL Backend (Windows)
- **File**: `engine/renderer/backend/wgl/WGLContext.cpp`
- **Platform**: Windows
- **Requirements**: Windows SDK (gdi32, user32)
- **Use Case**: Native Windows OpenGL context
- **Enabled By**: `ASTRAEUS_ENABLE_WGL=ON` (default on Windows)

### 3. Null Backend (Fallback)
- **File**: `engine/renderer/backend/NullContext.cpp`
- **Platform**: Any
- **Requirements**: None
- **Use Case**: Testing, builds without graphics support
- **Enabled By**: When no other backend is enabled

## Build Configuration

### Linux Build (default)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
**Result**: Uses EGL backend, links libEGL

### Windows Build (default)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
**Result**: Uses WGL backend, no EGL dependency

### Minimal Build (no graphics)
```bash
mkdir build && cd build
cmake .. -DASTRAEUS_ENABLE_EGL=OFF -DASTRAEUS_ENABLE_WGL=OFF
cmake --build .
```
**Result**: Uses null backend, no platform graphics libraries required

### Windows with ANGLE (advanced)
```bash
mkdir build && cd build
cmake .. -DASTRAEUS_ENABLE_EGL=ON -DASTRAEUS_ENABLE_WGL=OFF
cmake --build .
```
**Result**: Requires ANGLE EGL libraries on Windows

## CMake Options

- `ASTRAEUS_ENABLE_EGL` - Enable EGL backend (default: ON on Linux, OFF on Windows)
- `ASTRAEUS_ENABLE_WGL` - Enable WGL backend (default: ON on Windows, OFF on Linux)

## Architecture

```
GraphicsContext (interface)
    ├── EGLGraphicsContext (Linux/EGL)
    ├── WGLContext (Windows/WGL)
    └── NullContext (fallback)

GLRenderDevice
    └── Uses GraphicsContext* (via factory)
```

### Key Design Principles

1. **No EGL on Windows by default**: Windows builds don't require EGL headers or libraries
2. **Platform separation**: EGL headers only included in `egl/EGLContext.cpp`
3. **Clean interface**: `GraphicsContext` interface abstracts platform details
4. **Compile-time selection**: Backend chosen via CMake options and preprocessor defines

## Verification

Check which backend is active:
```bash
cmake .. 2>&1 | grep "Graphics Backend"
```

Output examples:
- `✓ EGL backend enabled (Linux/Unix or Windows with ANGLE)`
- `✓ WGL backend enabled (Windows)`
- `⚠ Warning: No graphics backend enabled - using null backend (no rendering)`

## Troubleshooting

### Linux: EGL not found
```bash
sudo apt-get install libegl1-mesa-dev libgl1-mesa-dev
```

### Windows: Build fails looking for EGL
Check that `ASTRAEUS_ENABLE_EGL=OFF` and `ASTRAEUS_ENABLE_WGL=ON` (should be automatic)

### No rendering output
Check CMake output - you might be using the null backend. Enable appropriate platform backend.
