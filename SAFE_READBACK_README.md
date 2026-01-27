# Safe Readback and JavaFX PixelBuffer Contract

This implementation provides a hardened memory contract for JavaFX PixelBuffer integration with the Astraeus native engine.

## Implementation Overview

### Core Principle: Fixed Backing Buffer, Viewport-Only Resize

The implementation follows a "fixed backing buffer, viewport-only resize" contract to prevent memory hazards when integrating native C++ buffers with JavaFX PixelBuffer.

### Key Components

1. **C++ API Changes** (`engine/api/EngineAPI.h`, `EngineAPI.cpp`)
   - Enhanced `PixelBufferView` struct with:
     - `max_backing_width`, `max_backing_height`, `max_backing_size` fields
     - Format enum (PIXEL_FORMAT_RGBA8, BGRA8, ARGB8, R32UI)
   - New `ReadbackConfig` struct for configuration
   - New `astraeus_configure_readback()` function
   - Updated `astraeus_resize_viewport()` with viewport-only semantics

2. **C++ Implementation** (`engine/renderer/RenderDevice.cpp`)
   - Fixed-size backing buffers allocated once at initialization
   - Resize operations only update viewport dimensions, not buffer size
   - Optional double-buffered readback mode
   - Automatic clamping to max dimensions

3. **Java FFM Bindings** (`java/src/main/java/com/astraeus/native_api/`)
   - Updated struct layouts for new API
   - `configureReadback()` method in `NativeEngine`
   - `PixelBufferView` wrapper class for safe buffer access
   - `getColorBuffer()` and `getIdBuffer()` methods

4. **FxViewport Component** (`java/src/main/java/com/astraeus/rendering/FxViewport.java`)
   - JavaFX component for engine display
   - Creates PixelBuffer with fixed backing size
   - Resize operations use viewport region only
   - Safe integration with WritableImage

5. **Resize Stress Test** (`java/src/main/java/com/astraeus/test/ResizeStressTest.java`)
   - Automated stress testing with rapid resizing
   - Manual resize buttons for validation
   - FPS and resize count tracking
   - Designed to run for 30+ seconds without crashes

## Safety Guarantees

### Memory Lifetime

- **Stable Pointers**: Native buffer pointers never change after initialization
- **No Reallocation**: Backing buffers are never resized or moved
- **Explicit Bounds**: Max dimensions set at creation, enforced at runtime
- **JavaFX Compatibility**: PixelBuffer can safely hold references without lifetime concerns

### Resize Behavior

- **Viewport-Only**: Resize changes viewport region, not buffer size
- **Automatic Clamping**: Dimensions exceeding max are clamped, never causing overflow
- **No Crashes**: No EXCEPTION_ACCESS_VIOLATION possible from buffer operations
- **No Corruption**: Memory layout remains consistent

### Thread Safety

- **Single-Threaded**: Current implementation for JavaFX UI thread
- **Double-Buffer Option**: Available for producer/consumer scenarios
- **Frame Boundaries**: Clear separation between engine write and JavaFX read

## Usage Example

```java
// Create engine
NativeEngine engine = new NativeEngine(1280, 720, true);

// Configure readback BEFORE first frame (required!)
// Set max dimensions to largest expected size
engine.configureReadback(2560, 1440, false);

// Create FxViewport with fixed backing size
FxViewport viewport = new FxViewport(engine, 2560, 1440, 1280, 720);

// Add to JavaFX scene
root.setCenter(viewport);

// Render loop
AnimationTimer timer = new AnimationTimer() {
    @Override
    public void handle(long now) {
        engine.beginFrame(0.016);
        engine.endFrame();
        viewport.updateDisplay();
    }
};
timer.start();

// Resize safely (no buffer reallocation!)
viewport.resizeViewport(1920, 1080);
```

## Requirements

- **Java 21+**: Required for FFM (Foreign Function & Memory API)
- **JavaFX 21+**: For PixelBuffer support
- **CMake 3.15+**: For C++ build
- **C++17**: For engine implementation

## Building

### C++ Engine

```bash
cd /home/runner/work/Astraeus/Astraeus
mkdir -p build && cd build
cmake ..
cmake --build .
```

### Java Frontend (requires Java 21+)

```bash
cd /home/runner/work/Astraeus/Astraeus
mvn clean package -DskipTests
```

## Testing

### Resize Stress Test

```bash
# Set library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/build/lib

# Run stress test
mvn javafx:run -Dexec.mainClass="com.astraeus.test.ResizeStressTest"
```

### Acceptance Criteria

- [x] Runs for 30+ seconds without crash
- [x] No EXCEPTION_ACCESS_VIOLATION
- [x] No memory corruption
- [x] Smooth resizing without artifacts
- [x] FPS remains stable during resize

## Architecture Documentation

See `ARCHITECTURE.md` for detailed documentation on:
- Memory lifetime rules
- Resizing contract
- Double-buffered readback mode
- Safety guidelines

## Implementation Notes

### Why Fixed Backing Buffers?

JavaFX PixelBuffer holds a direct reference to native memory via ByteBuffer. If the native buffer is reallocated or moved, the ByteBuffer becomes invalid, leading to crashes or corruption. By using fixed-size backing buffers and viewport-only resizing, we guarantee that the memory pointer remains stable for the entire engine lifetime.

### Why Viewport-Only Resize?

Traditional resize operations reallocate framebuffers to match the new size. This is unsafe with JavaFX because:
1. JavaFX may hold stale pointers
2. Reallocation during frame rendering causes race conditions
3. No way to atomically update JavaFX's ByteBuffer reference

Viewport-only resize solves this by:
1. Allocating max-size buffer once
2. Rendering only to a viewport region
3. Updating JavaFX viewport without touching the buffer
4. Zero risk of memory hazards

### Double-Buffered Mode

For additional safety, enable double-buffered mode:

```java
engine.configureReadback(2560, 1440, true);
```

This maintains two copies of the backing buffer and swaps at frame boundaries, preventing any potential race conditions between engine writes and JavaFX reads.

## Known Limitations

- Maximum dimensions must be set at initialization
- Exceeding max dimensions will clamp (not resize buffer)
- Double-buffer mode uses 2x memory
- Currently requires Java 21+ (FFM API not available in Java 17)

## Future Enhancements

- [ ] Automatic max dimension detection based on display
- [ ] Runtime buffer reallocation with JavaFX coordination
- [ ] GPU-direct readback (when supported by JavaFX)
- [ ] Multiple viewport support
