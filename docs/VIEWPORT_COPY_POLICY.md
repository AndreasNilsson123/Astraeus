# Viewport Copy Policy

**Version:** 1.0  
**Date:** 2026-01-31  
**Status:** Baseline Implementation

---

## Overview

This document defines the copy policy for transferring rendered image data from the native C++ engine to JavaFX for display. The current implementation uses a CPU-based copy as the baseline, with a documented upgrade path to GPU-based zero-copy when available.

---

## Baseline: CPU Copy

### Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    C++ Engine                           │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Render to GPU Framebuffer                       │  │
│  └──────────────────────────────────────────────────┘  │
│                          │                              │
│                          ▼                              │
│  ┌──────────────────────────────────────────────────┐  │
│  │  glReadPixels() / vkCmdCopyImageToBuffer()      │  │
│  │  Copy to CPU-accessible memory                   │  │
│  └──────────────────────────────────────────────────┘  │
│                          │                              │
└──────────────────────────┼──────────────────────────────┘
                           │
                           ▼
                  ┌─────────────────┐
                  │ Shared Memory   │
                  │ (Native Buffer) │
                  └─────────────────┘
                           │
                           ▼
┌──────────────────────────┼──────────────────────────────┐
│                    Java / JavaFX                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │  MemorySegment.ofAddress()                       │  │
│  │  Reinterpret native memory                       │  │
│  └──────────────────────────────────────────────────┘  │
│                          │                              │
│                          ▼                              │
│  ┌──────────────────────────────────────────────────┐  │
│  │  ByteBuffer (position=0, limit=capacity)         │  │
│  │  STABLE - never mutate position/limit/mark       │  │
│  └──────────────────────────────────────────────────┘  │
│                          │                              │
│                          ▼                              │
│  ┌──────────────────────────────────────────────────┐  │
│  │  PixelBuffer<ByteBuffer>                         │  │
│  │  Allocated once, never reallocated               │  │
│  └──────────────────────────────────────────────────┘  │
│                          │                              │
│                          ▼                              │
│  ┌──────────────────────────────────────────────────┐  │
│  │  WritableImage                                    │  │
│  │  JavaFX rendering pipeline                       │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Implementation Details

#### Native Side (C++)
- Render to GPU framebuffer
- Use `glReadPixels()` (OpenGL) or `vkCmdCopyImageToBuffer()` (Vulkan)
- Copy pixels to pre-allocated CPU-accessible memory
- Memory is allocated once at maximum viewport size
- Memory pointer remains stable for viewport lifetime

#### Java Side
- `MemorySegment.ofAddress()` wraps native memory address
- `asByteBuffer()` creates stable ByteBuffer view
- ByteBuffer given to JavaFX `PixelBuffer` (NEVER mutate after this)
- `WritableImage` wraps PixelBuffer
- `ImageView` displays WritableImage

### Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| GPU Render | O(pixels) | Standard GPU rendering |
| GPU → CPU Copy | O(pixels) | Blocking synchronous copy |
| Java Memory Wrap | O(1) | Zero-copy pointer wrapping |
| PixelBuffer Creation | O(1) | One-time allocation |
| JavaFX Display | O(pixels) | JavaFX rendering pipeline |

**Per-Frame Cost:**
- GPU → CPU copy: ~2-8ms for 1920x1080 @ 60Hz
- Memory wrapping: < 0.01ms (pointer arithmetic only)
- Total overhead: ~2-8ms per frame

**Memory Overhead:**
- Backing buffer: width × height × 4 bytes (BGRA8)
- Example: 2560×1440 = 14.75 MB
- Allocated once, never freed until viewport destroyed

### Advantages
- **Simple**: No complex GPU resource management
- **Portable**: Works on all platforms (OpenGL, Vulkan, Metal)
- **Predictable**: Deterministic performance characteristics
- **Debuggable**: CPU memory can be inspected/validated

### Disadvantages
- **Blocking**: GPU→CPU copy blocks render thread
- **Bandwidth**: Saturates PCIe bandwidth for large viewports
- **Latency**: Adds 1-2 frames of latency at high frame rates

---

## Future: GPU-Side Zero-Copy (PBO Path)

### Overview

For production environments with high frame rates or large viewports, a Pixel Buffer Object (PBO) based approach can eliminate the CPU copy.

### Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    C++ Engine                           │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Render to GPU Framebuffer                       │  │
│  └──────────────────────────────────────────────────┘  │
│                          │                              │
│                          ▼                              │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Copy to PBO (GPU→GPU)                           │  │
│  │  Asynchronous, non-blocking                      │  │
│  └──────────────────────────────────────────────────┘  │
│                          │                              │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Map PBO to CPU address space                    │  │
│  │  (zero-copy, GPU-accessible memory)              │  │
│  └──────────────────────────────────────────────────┘  │
└──────────────────────────┼──────────────────────────────┘
                           │
                           ▼
                  ┌─────────────────┐
                  │  Shared Memory  │
                  │ (PBO Mapping)   │
                  └─────────────────┘
                           │
                           ▼
┌──────────────────────────┼──────────────────────────────┐
│                    Java / JavaFX                         │
│  (Same as baseline from this point)                      │
└─────────────────────────────────────────────────────────┘
```

### Implementation Approach

#### Prerequisites
1. GPU supports persistent mapped buffers (OpenGL 4.4+, Vulkan)
2. Platform supports shared GPU/CPU memory (integrated GPUs or ReBAR)
3. Driver supports coherent memory access

#### Changes Required

**C++ Engine:**
```cpp
// Create persistent mapped PBO
glCreateBuffers(1, &pbo);
glNamedBufferStorage(pbo, size, nullptr, 
    GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
void* mappedPtr = glMapNamedBufferRange(pbo, 0, size,
    GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

// Each frame:
// 1. Render to FBO
// 2. Copy FBO → PBO (GPU-side, async)
glCopyImageSubData(...);
// 3. Return mapped pointer to Java (no sync needed if coherent)
```

**Java Side:**
- No changes to JavaFX integration
- `MemorySegment.ofAddress()` points to PBO mapped memory
- Rest of the pipeline remains identical

### Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| GPU Render | O(pixels) | Standard GPU rendering |
| GPU → PBO Copy | O(pixels) | Asynchronous, non-blocking |
| Java Memory Wrap | O(1) | Zero-copy pointer wrapping |
| Total overhead | < 1ms | Async copy + pointer wrap only |

**Expected Improvement:**
- Baseline: ~2-8ms per frame
- PBO: < 1ms per frame
- **Speedup: 2-8x reduction in copy overhead**

### Compatibility

| Platform | Support | Notes |
|----------|---------|-------|
| **Desktop Linux** | ✅ Yes | OpenGL 4.4+, Vulkan 1.0+ |
| **Desktop Windows** | ✅ Yes | OpenGL 4.4+, Vulkan 1.0+ |
| **Desktop macOS** | ⚠️ Limited | Metal only, requires special handling |
| **Integrated GPU** | ✅ Yes | Shared memory, best performance |
| **Discrete GPU** | ⚠️ Varies | Depends on ReBAR support |

---

## Upgrade Path

### Phase 1: Baseline (Current)
- ✅ CPU copy via `glReadPixels()`
- ✅ Stable ByteBuffer wrapping
- ✅ Works on all platforms

### Phase 2: PBO Detection
1. Add runtime GPU capability detection
2. Query for persistent mapped buffer support
3. Query for coherent memory support
4. Fall back to baseline if not supported

### Phase 3: PBO Implementation
1. Implement PBO allocation in renderer
2. Implement GPU→PBO copy in render passes
3. Expose PBO mapped pointer via C API
4. No Java changes required (same API surface)

### Phase 4: Validation
1. Benchmark PBO vs baseline performance
2. Test on multiple GPU vendors (NVIDIA, AMD, Intel)
3. Test on multiple platforms (Linux, Windows, macOS)
4. Document performance characteristics

### Phase 5: Production
1. Enable PBO by default where supported
2. Add configuration flag to force baseline mode
3. Log detected copy method on startup
4. Monitor performance in production

---

## Configuration

### Runtime Selection

```java
// Future API (not yet implemented)
EngineConfig config = new EngineConfig()
    .setCopyMode(CopyMode.AUTO)  // Auto-detect best mode
    // .setCopyMode(CopyMode.CPU)   // Force CPU copy
    // .setCopyMode(CopyMode.PBO)   // Force PBO (fail if unsupported)
    .setEnableValidation(true);

NativeEngine engine = new NativeEngine(config);
```

### Logging

```
[NativeEngine] Initializing viewport copy policy...
[NativeEngine] GPU: NVIDIA GeForce RTX 3080
[NativeEngine] OpenGL 4.6 detected
[NativeEngine] Persistent mapped buffers: SUPPORTED
[NativeEngine] Coherent memory: SUPPORTED
[NativeEngine] Copy mode: PBO (zero-copy)
```

---

## Debugging

### Verify Buffer Stability

Add this flag to enable buffer state checks:
```bash
java -Dastraeus.debug.assertBufferState=true ...
```

This logs warnings if ByteBuffer position/limit/mark are mutated after creation.

### Performance Profiling

```java
// Measure copy overhead
long start = System.nanoTime();
engine.endFrame();
viewport.updateDisplay();
long end = System.nanoTime();
System.out.println("Copy overhead: " + (end - start) / 1_000_000.0 + " ms");
```

---

## See Also

- [ARCHITECTURE.md](./ARCHITECTURE.md) - Overall system architecture
- [VIEWPORT_INTEGRATION.md](./VIEWPORT_INTEGRATION.md) - Viewport lifecycle
- [SAFE_READBACK_README.md](./SAFE_READBACK_README.md) - Memory safety guarantees

---

## Conclusion

The current baseline CPU copy implementation provides a robust, portable foundation for viewport rendering. The documented PBO upgrade path ensures we can optimize for performance in production without breaking the API or requiring application changes.

**Recommended Timeline:**
- **Now**: Ship with baseline (proven, simple, works everywhere)
- **Q2 2026**: Implement PBO detection and fallback
- **Q3 2026**: Production testing and validation
- **Q4 2026**: Enable PBO by default

---

*End of Document*
