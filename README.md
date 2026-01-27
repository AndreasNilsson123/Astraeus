# Astraeus

**A Professional 3D Visualization Engine for High-Performance Data Visualization**

Astraeus is a scalable 3D visualization engine designed for high-performance visualization of externally simulated data. It combines a high-performance C++ rendering core with a flexible Java/JavaFX frontend, connected through the Foreign Function & Memory (FFM) API.

## Architecture

### Core Principles

- **Clear Ownership**: C++ owns memory, state, and GPU objects; Java never reallocates native memory
- **Stable ABI**: Small C-style API surface for FFM with opaque handles and POD structs only
- **Data-Oriented Design**: Structure of Arrays (SoA) layouts with handle-based entities
- **Render-Graph Based**: Pass-based, extensible renderer supporting multiple outputs
- **Visualization-First**: No physics engine; focuses on tracks, volumes, overlays, and labels
- **Tool-Friendly**: Deep JavaFX integration with zero-copy readback where needed

### Components

#### C++ Engine Core (`engine/`)

The native engine handles all performance-critical operations:

- **core/**: EngineContext, services, diagnostics
- **renderer/**: RenderDevice (backend abstraction), RenderGraph (pass management)
- **scene/**: World management, entities, transforms, visibility
- **geometry/**: Mesh representation, procedural geometry
- **ingest/**: External data ingestion and synchronization
- **assets/**: Model and texture loading, GPU upload
- **api/**: Stable C ABI for FFM (`EngineAPI.h`)

#### Java Frontend (`java/`)

The Java layer provides tooling and UI:

- **native/**: FFM bindings and NativeEngine wrapper
- **rendering/**: JavaFX surfaces, viewport management, overlays
- **tools/**: Scene inspector, debug panels, property browsers
- **workflow/**: Scenario control, playback, state management
- **ui/**: JavaFX layouts and user interaction

### Integration Layer

The C++ and Java components communicate exclusively through FFM (Foreign Function & Memory API):

- No callbacks from C++ into Java
- No Java object references in native code
- Native memory allocated once and exposed via `MemorySegment`
- Explicit lifecycle management

## Building

### Prerequisites

- **C++ Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake**: 3.15 or later
- **Java**: JDK 21+ (for FFM support)
- **Maven**: 3.6+ (for Java build)

### Build C++ Engine

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

The shared library (`libastraeus.so` on Linux, `astraeus.dll` on Windows, `libastraeus.dylib` on macOS) will be created in `build/lib/`.

### Build Java Frontend

**Note**: The Java FFM bindings require Java 21+ (FFM was finalized in Java 22, available as preview in Java 19-21). If you have an older Java version, the Java components will not compile. The C++ engine can still be built and used independently.

```bash
mvn clean package
```

This compiles the Java sources and creates a JAR in `target/`.

### Running

Before running the Java application, ensure the native library is in your library path:

**Linux/macOS:**
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/build/lib
mvn javafx:run
```

**Windows:**
```cmd
set PATH=%PATH%;%CD%\build\lib
mvn javafx:run
```

## API Overview

### C API (EngineAPI.h)

The stable C ABI provides the following operations:

- **Lifecycle**: `astraeus_create_engine()`, `astraeus_destroy_engine()`
- **Rendering**: `astraeus_begin_frame()`, `astraeus_end_frame()`, `astraeus_resize_viewport()`
- **Scene**: `astraeus_create_entity()`, `astraeus_destroy_entity()`, `astraeus_set_entity_transform()`
- **Camera**: `astraeus_set_camera()`, `astraeus_set_camera_projection()`
- **Data**: `astraeus_ingest_data()` for external simulation data
- **Readback**: `astraeus_get_color_buffer()`, `astraeus_get_id_buffer()`
- **Picking**: `astraeus_pick()` for entity selection

### Java API (NativeEngine)

High-level wrapper providing safe access to native functionality:

```java
try (NativeEngine engine = new NativeEngine(1280, 720, true)) {
    int entityId = engine.createEntity();
    
    engine.beginFrame(0.016);
    // Render
    engine.endFrame();
}
```

## Extending the Engine

### Adding New Render Passes

1. Create a new class inheriting from `RenderPass` in C++
2. Implement `initialize()`, `execute()`, and `on_resize()`
3. Register the pass with the `RenderGraph`

### Adding New Ingest Schemas

1. Define your data format in `IngestManager`
2. Implement parsing logic to convert to scene entities
3. No changes needed to renderer or Java layer

### Adding New Java Tools

1. Create new tool classes in `java/tools/`
2. Use `NativeEngine` API to query/modify state
3. Integrate with JavaFX UI
4. No changes needed to C++ engine

## Design Decisions

### Why FFM Instead of JNI?

- **Type Safety**: FFM provides compile-time type checking
- **Performance**: Zero-copy memory access with `MemorySegment`
- **Modern**: Standardized in Java 21+, future-proof
- **Simplicity**: No JNI boilerplate or header generation

### Why C++ for the Core?

- **Performance**: Direct GPU access, minimal overhead
- **Control**: Explicit memory management for large datasets
- **Backend Flexibility**: Easy to swap OpenGL → Vulkan → DirectX
- **Tooling**: Mature profiling and debugging tools

### Why Java for the Frontend?

- **UI Framework**: JavaFX provides rich, cross-platform UI
- **Rapid Development**: Quick iteration on tools and inspectors
- **Ecosystem**: Maven, extensive libraries
- **Safe by Default**: Automatic memory management for UI code

## Future Roadmap

- [ ] OpenGL backend implementation
- [ ] Vulkan backend for high-performance rendering
- [ ] Advanced render passes (PBR, shadows, post-processing)
- [ ] Multi-threaded scene updates
- [ ] Streaming geometry system
- [ ] Advanced picking with spatial queries
- [ ] Timeline editor for scenario playback
- [ ] Plugin system for custom visualizations

## License

See [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please ensure:

1. C++ code follows the existing style (see `.clang-format` if present)
2. Java code follows standard Java conventions
3. New features include documentation
4. The stable C ABI remains backward-compatible

## Contact

For questions or issues, please open a GitHub issue.