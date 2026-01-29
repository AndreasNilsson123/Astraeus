# Astraeus Telemetry System

## Overview

The Astraeus telemetry system provides high-performance frame and pass-level performance metrics with minimal overhead. It follows the engine's design principles of ZERO overhead when disabled and ≤1-2% overhead when enabled.

## Architecture

### Components

1. **Telemetry.hpp** - Core telemetry class with frame and pass tracking
2. **EngineContext** - Integrates telemetry into the engine lifecycle
3. **RenderGraph** - Per-pass timing using RAII timers
4. **EngineAPI.h** - C FFM interface for Java integration

## Key Features

### Zero-Overhead Design
- **Compile-time checks**: All telemetry calls check `is_enabled()` and become no-ops when disabled
- **No allocations per frame**: Fixed-size ring buffer and pass arrays
- **RAII timers**: Automatic timing with scope-based cleanup
- **Cache-friendly**: Compact POD structs with explicit padding

### Ring Buffer History
- Stores last 300 frames (~5 seconds at 60 FPS)
- Fixed memory allocation (no dynamic growth)
- Circular buffer with automatic wrap-around

### Per-Pass Timing
- Up to 16 render passes tracked per frame
- Automatic timing using PassTimer RAII class
- Pass names stored inline (32 bytes)

## Data Structures

### TelemetryFrameStats (C FFM)
```c
typedef struct {
    uint64_t frame_number;
    double cpu_time_ms;
    double gpu_time_ms;      // Placeholder (requires GPU query extension)
    double total_time_ms;
    uint32_t draw_calls;
    uint32_t triangle_count;
} TelemetryFrameStats;
```

### PassTiming
```cpp
struct PassTiming {
    char name[32];           // Pass name (null-terminated)
    double duration_ms;
    bool active;
    uint8_t _padding[7];     // Explicit alignment
};
```

## C++ Usage

### Basic Frame Timing
```cpp
// In EngineContext
telemetry_->begin_frame(frame_count_);

// ... render work ...

telemetry_->end_frame(draw_calls, triangle_count);
```

### Per-Pass Timing (Manual)
```cpp
uint32_t pass_idx = telemetry->begin_pass("MyPass");
// ... pass rendering ...
telemetry->end_pass(pass_idx);
```

### Per-Pass Timing (RAII - Recommended)
```cpp
{
    PassTimer timer(telemetry, "MyPass");
    // ... pass rendering ...
}  // Automatically records timing
```

### Getting Statistics
```cpp
// Current frame stats
const auto& stats = telemetry->get_current_stats();
std::cout << "CPU: " << stats.cpu_time_ms << " ms\n";

// Historical data
Telemetry::FrameStats history[100];
uint32_t count = telemetry->get_history(history, 100);

// Pass timing
const auto* pass = telemetry->get_pass_timing(0);
if (pass) {
    std::cout << pass->name << ": " << pass->duration_ms << " ms\n";
}
```

## C API (FFM Integration)

### Enable/Disable Telemetry
```c
// Enable telemetry (default: enabled)
astraeus_enable_telemetry(engine, true);

// Check if enabled
bool enabled = astraeus_is_telemetry_enabled(engine);
```

### Get Frame Statistics
```c
TelemetryFrameStats stats;
astraeus_get_telemetry_frame_stats(engine, &stats);

printf("Frame %lu: CPU=%.2f ms, Draw calls=%u\n", 
       stats.frame_number, stats.cpu_time_ms, stats.draw_calls);
```

### Get Historical Data
```c
#define HISTORY_SIZE 300
TelemetryFrameStats history[HISTORY_SIZE];
uint32_t count = astraeus_get_telemetry_history(engine, history, HISTORY_SIZE);

for (uint32_t i = 0; i < count; i++) {
    printf("Frame %lu: %.2f ms\n", history[i].frame_number, history[i].cpu_time_ms);
}
```

### Get Pass Timing
```c
uint32_t pass_count = astraeus_get_pass_count(engine);
for (uint32_t i = 0; i < pass_count; i++) {
    char name[64];
    double time_ms;
    if (astraeus_get_pass_timing(engine, i, name, sizeof(name), &time_ms)) {
        printf("Pass '%s': %.3f ms\n", name, time_ms);
    }
}
```

## Java Integration Example

```java
// Enable telemetry
engineAPI.astraeus_enable_telemetry(engine, true);

// Get current frame stats
var stats = new TelemetryFrameStats();
engineAPI.astraeus_get_telemetry_frame_stats(engine, stats);
System.out.printf("CPU: %.2f ms, Draw calls: %d%n", 
                  stats.cpu_time_ms(), stats.draw_calls());

// Get history for graphing
var history = new TelemetryFrameStats[300];
int count = engineAPI.astraeus_get_telemetry_history(engine, history, 300);
// Plot history data...

// Get pass timings
int passCount = engineAPI.astraeus_get_pass_count(engine);
for (int i = 0; i < passCount; i++) {
    var name = new byte[64];
    var timeMs = new double[1];
    if (engineAPI.astraeus_get_pass_timing(engine, i, name, 64, timeMs)) {
        System.out.printf("Pass '%s': %.3f ms%n", 
                          new String(name).trim(), timeMs[0]);
    }
}
```

## Performance Characteristics

### Overhead When Enabled
- Per-frame overhead: ~0.5-1% (mostly timer queries)
- Per-pass overhead: ~10-50 nanoseconds (RAII + chrono)
- Memory overhead: ~50 KB (ring buffer + pass data)

### Overhead When Disabled
- **ZERO** - All calls become empty inline no-ops
- Compiler optimizes away all telemetry code paths
- No runtime checks beyond initial `is_enabled()` branch

## Implementation Notes

### Ring Buffer Design
```cpp
// Fixed-size circular buffer
std::array<FrameStats, HISTORY_SIZE> history_;
uint32_t history_head_;   // Next write position
uint32_t history_count_;  // Valid entries (≤ HISTORY_SIZE)
```

### Timing Precision
- Uses `std::chrono::high_resolution_clock`
- Nanosecond precision, converted to milliseconds for output
- GPU timing requires extension (placeholder for now)

### Thread Safety
- Current implementation: **Single-threaded only**
- All telemetry calls must occur on the render thread
- Future: Add optional mutex for multi-threaded profiling

## Future Enhancements

### Planned Features
1. **GPU Query Integration**
   - OpenGL: `glQueryCounter` with `GL_TIMESTAMP`
   - Vulkan: `vkCmdWriteTimestamp`
   - Add async query result retrieval

2. **Memory Profiling**
   - Track GPU memory usage (textures, buffers)
   - Track CPU memory allocations

3. **Custom Markers**
   - User-defined timing regions
   - Hierarchical profiling (nested timers)

4. **Export/Import**
   - JSON export for offline analysis
   - Chrome Tracing format support

## Integration Checklist

### C++ Engine Side
- [x] Create Telemetry.hpp with core functionality
- [x] Integrate into EngineContext lifecycle
- [x] Add per-pass timing to RenderGraph
- [x] Update all RenderPass implementations with get_name()

### C API Side
- [x] Add TelemetryFrameStats struct to EngineAPI.h
- [x] Implement telemetry FFM functions
- [x] Test C API compilation

### Java Side
- [ ] Generate FFM bindings for telemetry functions
- [ ] Create TelemetryPanel JavaFX component
- [ ] Add real-time graph visualization
- [ ] Add pass timing breakdown display

## Troubleshooting

### High Overhead When Enabled
- Ensure telemetry is only enabled when actively profiling
- Check that PassTimer is not used in tight loops
- Verify no unnecessary history retrieval per frame

### Missing Pass Timings
- Verify RenderPass::get_name() is implemented
- Check that telemetry is enabled before passes execute
- Ensure pass count doesn't exceed MAX_PASSES (16)

### Incorrect Timing Values
- Verify high_resolution_clock is stable on your platform
- Check for clock rollover on long-running sessions
- Ensure no threads are interfering with timing

## References

- **Design Doc**: docs/ARCHITECTURE.md (Task C1)
- **API Reference**: engine/api/EngineAPI.h
- **Implementation**: engine/core/Telemetry.hpp
- **Usage Example**: java/src/main/java/.../TelemetryPanel.java (TODO)
