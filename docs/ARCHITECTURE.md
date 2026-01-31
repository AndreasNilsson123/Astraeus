# Astraeus Architecture

## Overview

Astraeus is designed as a **professional, scalable 3D visualization engine** for high-performance visualization of externally simulated data. The architecture separates concerns cleanly between performance-critical rendering (C++) and flexible tooling/UI (Java), connected via FFM.

**See also:**
- [DEPENDENCIES.md](DEPENDENCIES.md) - Dependency footprint and platform abstraction policy

## Architectural Layers

```
┌─────────────────────────────────────────────────┐
│         Java Frontend (JavaFX UI)               │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │  Tools   │  │ Workflow │  │   UI     │     │
│  └──────────┘  └──────────┘  └──────────┘     │
│  ┌──────────────────────────────────────┐      │
│  │      Rendering (Viewport Mgmt)       │      │
│  └──────────────────────────────────────┘      │
│  ┌──────────────────────────────────────┐      │
│  │   Native Bindings (FFM/JNI Bridge)   │      │
│  └──────────────────────────────────────┘      │
└─────────────────────────────────────────────────┘
                      │ FFM API
                      ▼
┌─────────────────────────────────────────────────┐
│            C API Layer (EngineAPI.h)            │
│  - Opaque handles (EngineHandle)                │
│  - POD structs (FrameStats, PickResult)         │
│  - C linkage for ABI stability                  │
└─────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────┐
│         C++ Engine Core (Astraeus)              │
│  ┌──────────────────────────────────────┐      │
│  │      Engine Context (Coordinator)     │      │
│  └──────────────────────────────────────┘      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │Renderer  │  │  Scene   │  │  Ingest  │     │
│  │(RGraph)  │  │ (World)  │  │ (Manager)│     │
│  └──────────┘  └──────────┘  └──────────┘     │
│  ┌──────────┐  ┌──────────┐                    │
│  │ Geometry │  │  Assets  │                    │
│  └──────────┘  └──────────┘                    │
└─────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────┐
│              Graphics Backend                   │
│         (OpenGL, Vulkan, etc.)                  │
└─────────────────────────────────────────────────┘
```

## Key Design Patterns

### 1. Handle-Based Entity System

Entities are represented as opaque 32-bit handles, not pointers:

```cpp
uint32_t entity_id = astraeus_create_entity(engine);
astraeus_set_entity_transform(engine, entity_id, ...);
```

**Benefits:**
- Type-safe across language boundary
- Can be serialized/deserialized
- Stable references (no pointer invalidation)
- Generation indices can be added later

### 2. Data-Oriented Design (SoA)

Component data stored in Structure-of-Arrays layout:

```cpp
class World {
    std::unordered_map<uint32_t, Transform> transforms_;
    std::unordered_map<uint32_t, Renderable> renderables_;
    // etc.
};
```

**Benefits:**
- Cache-friendly iteration
- Easy to parallelize updates
- Compact memory layout
- SIMD-friendly

### 3. Render Graph

Rendering organized as a directed acyclic graph of passes:

```
┌──────────┐    ┌──────────┐    ┌──────────┐
│  Grid    │───▶│  Scene   │───▶│  UI      │
└──────────┘    └──────────┘    └──────────┘
                     │
                     ▼
                ┌──────────┐
                │  Picking │
                └──────────┘
```

**Benefits:**
- Explicit dependencies
- Easy to add/remove passes
- Automatic resource synchronization
- Multiple output buffers (color, ID, depth)

### 4. Stable ABI Boundary

Only POD types and opaque pointers cross the FFM boundary:

**Allowed:**
```c
typedef struct { float x, y, z; } Vec3;
typedef struct AstraeusEngine* EngineHandle;
void astraeus_func(EngineHandle h, Vec3 v);
```

**Not Allowed:**
```cpp
// No C++ types, no STL, no callbacks
std::vector<Entity> get_entities();  // ❌
void set_callback(void (*cb)());     // ❌
```

### 5. Zero-Copy Readback

Framebuffer data exposed directly to Java via `MemorySegment`:

```java
PixelBufferView view = engine.getColorBuffer();
MemorySegment buffer = view.data();  // No copy!
// Java can read pixel data directly from GPU-mapped memory
```

## Data Flow

### Ingestion Flow

```
External Simulation
        │
        ▼
  IngestManager (C++)
        │
        ├─ Parse data format
        ├─ Create/update entities
        ├─ Set transforms
        └─ Update scene state
        │
        ▼
     World (Scene)
        │
        ▼
   RenderGraph
        │
        ▼
    Framebuffer
        │
        ▼
   Java (Display)
```

### Rendering Flow

```
Java calls beginFrame()
        │
        ▼
  EngineContext::begin_frame()
        │
        ▼
  RenderDevice::begin_frame()
        │
        ▼
  RenderGraph::execute()
        │
        ├─ For each pass:
        │   ├─ Bind framebuffer
        │   ├─ Set uniforms
        │   ├─ Draw entities
        │   └─ Next pass
        │
        ▼
  RenderDevice::end_frame()
        │
        ▼
Java calls endFrame()
        │
        ▼
  Present to screen
```

### Picking Flow

```
User clicks viewport (x, y)
        │
        ▼
Java calls pick(x, y)
        │
        ▼
RenderDevice reads ID buffer[x, y]
        │
        ├─ entity_id from R channel
        ├─ depth from depth buffer
        └─ Unproject to world coords
        │
        ▼
Return PickResult
        │
        ▼
Java highlights selected entity
```

## Extensibility Points

### Adding a New Render Pass

1. Inherit from `RenderPass`:
   ```cpp
   class MyPass : public RenderPass {
       bool initialize(RenderDevice* device) override;
       void execute(RenderDevice* device, World* world) override;
       void on_resize(uint32_t w, uint32_t h) override;
   };
   ```

2. Register in `RenderGraph`:
   ```cpp
   render_graph->add_pass(std::make_unique<MyPass>());
   ```

### Adding a New Ingest Format

1. Define format ID in `IngestManager`:
   ```cpp
   enum class DataFormat {
       Custom = 0,
       JSON = 1,
       Binary = 2,
       MyNewFormat = 3  // Add here
   };
   ```

2. Implement parser:
   ```cpp
   bool IngestManager::ingest(const void* data, uint32_t size, uint32_t format) {
       switch (format) {
           case 3: return parse_my_format(data, size);
       }
   }
   ```

### Adding a New Java Tool

1. Create tool class:
   ```java
   public class MyTool {
       private NativeEngine engine;
       
       public void update() {
           int entity = engine.createEntity();
           // ...
       }
   }
   ```

2. Integrate with UI:
   ```java
   MyTool tool = new MyTool(engine);
   Button button = new Button("My Tool");
   button.setOnAction(e -> tool.update());
   ```

## Performance Considerations

### Memory Allocation

- **C++ owns all long-lived allocations**
- **Java never resizes native buffers**
- Use memory pools for frequent allocations
- Pre-allocate buffers at known upper bounds

### Readback Buffer Lifetime Rules

**CRITICAL FOR JAVAFX INTEGRATION:**

The readback buffers (color and ID buffers) follow a strict "fixed backing size, viewport-only resize" contract:

1. **Fixed Backing Buffer**: Allocated once at engine initialization with maximum expected dimensions
   - Default: 2560x1440 (can be configured via `astraeus_configure_readback()`)
   - Memory pointer remains STABLE for engine lifetime
   - Never reallocated, never moved
   
2. **Viewport-Only Resize**: Resize operations only update the viewport region
   - `astraeus_resize_viewport()` changes viewport dimensions, NOT backing buffer
   - PixelBufferView returns current viewport dimensions and stable backing pointer
   - JavaFX PixelBuffer wraps the full backing buffer and updates viewport region only
   
3. **Safety Guarantees**:
   - No `EXCEPTION_ACCESS_VIOLATION` due to buffer reallocation
   - No memory corruption from pointer invalidation
   - No race conditions from buffer swapping (optional double-buffer mode available)
   - JavaFX can safely hold references to native memory without lifetime concerns

**Usage Example:**

```java
// Create engine with initial size
NativeEngine engine = new NativeEngine(1280, 720, true);

// Configure readback with maximum expected size (BEFORE first frame)
engine.configureReadback(2560, 1440, false);

// Create FxViewport with fixed backing size
FxViewport viewport = new FxViewport(engine, 2560, 1440, 1280, 720);

// Resize viewport (SAFE - only changes viewport region)
viewport.resizeViewport(1920, 1080);  // No buffer reallocation!
viewport.resizeViewport(1280, 720);   // No buffer reallocation!
viewport.resizeViewport(2560, 1440);  // No buffer reallocation!
```

**Double-Buffered Mode:**

For extra safety in multi-threaded scenarios, enable double-buffered readback:

```java
engine.configureReadback(2560, 1440, true);
```

This maintains two copies of the backing buffer and swaps between them at frame boundaries, preventing race conditions between engine writes and JavaFX reads.

### Resizing Contract

**Viewport Resize (Safe):**
- Changes viewport dimensions only
- Backing buffer remains fixed size
- Memory pointer unchanged
- JavaFX PixelBuffer remains valid
- Can resize up to maxWidth x maxHeight without issues

**DO NOT:**
- ❌ Reallocate backing buffers after initialization
- ❌ Change memory pointers held by JavaFX
- ❌ Exceed max backing dimensions (will be clamped)
- ❌ Call `configure_readback()` after initialization

**Thread Safety:**
- Single-threaded access (Java UI thread calls engine)
- Optional double-buffer mode for producer/consumer pattern
- Engine write completes before JavaFX read (frame boundaries)

### Threading

Current design is single-threaded, but prepared for:
- Render thread (C++)
- Update thread (C++)
- UI thread (Java, separate)

Future: Use job system for parallel scene updates.

### GPU Optimization

- Batch draw calls per material/shader
- Use instancing for repeated geometry
- Minimize state changes
- Use indirect drawing for large entity counts

## Security Considerations

### FFM Safety

- All native pointers wrapped in opaque handles
- Java cannot dereference arbitrary memory
- Bounds checking on all array accesses
- Resource cleanup guaranteed via `AutoCloseable`

### Memory Layout Portability

**Important**: The Java FFM bindings use manually-defined struct layouts that may not be portable across all platforms. The current implementation assumes x64 Linux/Windows with standard alignment.

For production use, consider:
1. Using `jextract` tool to generate layouts from C headers automatically
2. Testing on target platforms to verify layout compatibility
3. Using runtime layout queries if available
4. Documenting supported platforms explicitly

The C API uses standard C types (`uint32_t`, `float`, etc.) which should be consistent, but padding and alignment can vary.

### Input Validation

- All C API functions validate handles
- Null checks on all pointer parameters
- Size checks on buffer operations
- Format validation in ingest pipeline

## Testing Strategy

### C++ Tests (Future)

- Unit tests for each subsystem
- Integration tests for full pipeline
- Benchmark tests for performance regression

### Java Tests (Future)

- Unit tests for tool logic
- UI tests with TestFX
- Integration tests with mock engine

## Deployment

### Library Distribution

- C++ shared library per platform (`.so`, `.dll`, `.dylib`)
- Java JAR with all classes
- Native library loading via `System.loadLibrary()`

### Platform Support

- **Linux**: Primary development target
- **Windows**: Supported with MSVC or MinGW
- **macOS**: Supported with Clang

## Future Enhancements

### Short Term
- Implement OpenGL backend
- Add more render passes (grid, axes)
- Implement actual picking
- Add frame stats display in UI

### Medium Term
- Vulkan backend for high performance
- Multi-threaded rendering
- Streaming geometry system
- Advanced camera controls

### Long Term
- Plugin system for custom passes
- Python bindings for data ingestion
- Distributed rendering
- VR/AR support

---

## Module Structure and Dependencies

### C++ Engine Modules

```
engine/
├── api/              # C API boundary (FFM interface)
├── core/             # Engine context, telemetry, utilities
├── platform/         # Platform abstraction (Win32, Linux)
├── renderer/         # Render device, render graph, passes
│   ├── backend/      # Graphics context (WGL, EGL, Null)
│   ├── passes/       # Render passes (grid, mesh, picking)
│   └── opengl/       # OpenGL-specific render device
├── scene/            # World, entities, transforms, camera
├── ingest/           # Data ingestion from external sources
├── assets/           # Mesh loading, GPU upload, asset management
├── geometry/         # Geometry utilities
└── third_party/      # External dependencies (GLAD)
```

**Dependency Rules:**
- `api/` depends on: `core/`, `renderer/`, `scene/`, `ingest/`
- `core/` depends on: `platform/` only
- `platform/` depends on: nothing (standalone)
- `renderer/` depends on: `core/`, `platform/`, `scene/`
- `scene/` depends on: `core/`
- `ingest/` depends on: `core/`, `scene/`
- `assets/` depends on: `core/`, `renderer/`

### Java Modules (Logical Structure)

```
java/src/main/java/com/astraeus/
├── native_api/       # FFM bindings, no dependencies
│   ├── EngineAPI.java
│   ├── NativeEngine.java
│   └── structs/
├── ui/               # JavaFX application
│   ├── AstraeusApp.java
│   └── MainWindow.java
├── rendering/        # Viewport management, JavaFX integration
│   ├── Viewport.java
│   └── ViewportContainer.java
├── tools/            # Debugging tools, schema generation
│   ├── telemetry/
│   └── inspector/
├── scene/            # Scene outliner, hierarchy viewer
├── workflow/         # Workflow orchestration
└── util/             # Utilities
```

**Dependency Rules:**
- `native_api/` depends on: JDK only (no JavaFX)
- `ui/` depends on: `native_api/`, JavaFX
- `rendering/` depends on: `native_api/`, JavaFX
- `tools/` depends on: `native_api/` (may have optional YAML/JSON deps)
- `scene/` depends on: `native_api/`, JavaFX
- Tool dependencies (YAML, JSON) must NOT leak into `ui/` or runtime

### Platform Abstraction Details

The `engine/platform/` module provides a **minimal, stable interface** for platform-specific operations. See [DEPENDENCIES.md](DEPENDENCIES.md#platform-module-interface) for full details.

**Key Functions:**
```cpp
namespace astraeus::platform {
    void init();                         // Initialize platform
    uint64_t monotonic_time_ns();        // High-resolution timing
    void* load_gl_proc(const char* name); // OpenGL function loading
    void set_thread_name(const char* name); // Thread debugging
    size_t get_page_size();              // Memory utilities
}
```

**Platform-Specific Files:**
- `Platform.hpp` - Public interface (no platform types)
- `Win32/Win32Headers.hpp` - Centralized `<windows.h>` include
- `Win32/Win32Platform.cpp` - Windows implementation
- `Linux/X11Headers.hpp` - Centralized X11 headers (stub)
- `Linux/LinuxPlatform.cpp` - Linux implementation
- `GL/GLHeaders.hpp` - Centralized OpenGL/GLAD headers

**Rules:**
- Only `engine/platform/` may contain `#ifdef _WIN32`, `#ifdef __linux__`, etc.
- Only `engine/platform/` may include `<windows.h>`, X11 headers, etc.
- All other engine code must use `platform::*` functions

For comprehensive dependency and platform abstraction rules, see [DEPENDENCIES.md](DEPENDENCIES.md).
