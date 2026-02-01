# Telemetry UI Components

This package provides JavaFX UI components for displaying real-time performance telemetry from the Astraeus visualization engine.

## Components

### 1. TelemetryOverlay
**Location:** `com.astraeus.tools.TelemetryOverlay`

Lightweight HUD overlay for the viewport that displays:
- FPS (frames per second)
- CPU time (ms)
- GPU time (ms, or "N/A" if not available)
- Draw calls
- Triangle count

**Features:**
- Semi-transparent dark background
- Top-right corner positioning
- NO per-frame allocations (reuses Label instances)
- Mouse-transparent (doesn't block viewport interaction)
- Toggle visibility with `toggleVisible()`

**Usage:**
```java
TelemetryOverlay overlay = new TelemetryOverlay();
overlay.setVisible(true);

// In render loop:
TelemetryFrameStats stats = engine.getTelemetryStats();
overlay.update(stats);
```

### 2. TelemetryPane
**Location:** `com.astraeus.tools.TelemetryPane`

Detailed telemetry panel for docking/tooling windows that displays:
- Overall statistics (frame number, FPS, CPU/GPU time, draw calls, triangles, pass count)
- Per-pass timing breakdown table (sorted by time)
- Enable/disable telemetry checkbox

**Features:**
- Professional table-based layout
- Per-pass breakdown with percentages
- Automatic sorting by time (descending)
- NO per-frame allocations (reuses TableView rows)
- Integrated telemetry toggle control

**Usage:**
```java
TelemetryPane pane = new TelemetryPane(engine);

// In update loop (throttled to ~30 Hz):
pane.update();
```

## FFM Bindings

### NativeEngine Additions
The following telemetry methods have been added to `NativeEngine.java`:

#### Methods
- `void enableTelemetry(boolean enabled)` - Enable/disable telemetry collection
- `boolean isTelemetryEnabled()` - Check if telemetry is enabled
- `TelemetryFrameStats getTelemetryStats()` - Get current frame statistics
- `List<TelemetryFrameStats> getTelemetryHistory(int maxFrames)` - Get history ring buffer
- `int getPassCount()` - Get number of render passes
- `PassTiming getPassTiming(int passIndex)` - Get timing for specific pass

#### Classes
- `TelemetryFrameStats` - Immutable snapshot of frame performance metrics
  - `long getFrameNumber()`
  - `double getCpuTimeMs()`
  - `double getGpuTimeMs()`
  - `double getTotalTimeMs()`
  - `int getDrawCalls()`
  - `int getTriangleCount()`
  - `int getPassCount()`
  - `double getFPS()` - Calculated from total time

- `PassTiming` - Render pass timing information
  - `String getName()`
  - `double getTimeMs()`

### EngineBindings Additions
The following native bindings have been added to `EngineBindings.java`:

#### Memory Layouts
- `TELEMETRY_FRAME_STATS_LAYOUT` - Struct layout for `TelemetryFrameStats`

#### Function Descriptors and Method Handles
- `ENABLE_TELEMETRY`
- `IS_TELEMETRY_ENABLED`
- `GET_TELEMETRY_FRAME_STATS`
- `GET_TELEMETRY_HISTORY`
- `GET_PASS_COUNT`
- `GET_PASS_TIMING`

## Integration Guide

### Basic Integration

#### 1. Add Overlay to Viewport
```java
// In your viewport setup:
TelemetryOverlay overlay = new TelemetryOverlay();

StackPane viewportStack = new StackPane();
viewportStack.getChildren().addAll(yourViewport, overlay);
StackPane.setAlignment(overlay, Pos.TOP_RIGHT);
StackPane.setMargin(overlay, new Insets(10));

// Toggle with keyboard shortcut:
scene.setOnKeyPressed(e -> {
    if (e.getCode() == KeyCode.F1) {
        overlay.toggleVisible();
    }
});
```

#### 2. Add Detailed Panel to Tooling Window
```java
TelemetryPane telemetryPane = new TelemetryPane(engine);
toolsTabPane.getTabs().add(new Tab("Telemetry", telemetryPane));
```

#### 3. Update in Render Loop (with Throttling)
```java
private static final long TELEMETRY_UPDATE_NS = 33_000_000L; // 30 Hz
private long lastTelemetryUpdate = 0;

// In your AnimationTimer.handle():
if (engine.isTelemetryEnabled() && 
    (now - lastTelemetryUpdate) >= TELEMETRY_UPDATE_NS) {
    
    TelemetryFrameStats stats = engine.getTelemetryStats();
    overlay.update(stats);
    telemetryPane.update();
    
    lastTelemetryUpdate = now;
}
```

#### 4. Enable/Disable Telemetry
```java
// Enable at startup:
engine.enableTelemetry(true);

// Toggle with button:
toggleButton.setOnAction(e -> {
    engine.enableTelemetry(!engine.isTelemetryEnabled());
});
```

## Performance Considerations

### Update Throttling
**IMPORTANT:** Always throttle telemetry UI updates to ~30 Hz, not every frame.

Reasons:
- Reduces JavaFX layout overhead
- Human eyes can't perceive 60 Hz UI text updates
- Telemetry data is statistical, not critical for every frame
- Prevents UI thread saturation

### Zero Native Overhead When Disabled
When telemetry is disabled via `engine.enableTelemetry(false)`:
- Native C++ telemetry collection has **ZERO** overhead
- No timing measurements are taken
- No data is recorded
- Frame rate is unaffected

### UI Component Efficiency
Both `TelemetryOverlay` and `TelemetryPane`:
- Reuse existing JavaFX nodes (NO per-frame allocations)
- Only update text content via `setText()`
- Minimize layout passes
- Use efficient JavaFX patterns

### Recommended Update Pattern
```java
// Update at 30 Hz (33ms intervals)
private static final long UPDATE_INTERVAL_NS = 33_000_000L;
private long lastUpdate = 0;

void onFrame(long now) {
    // Always render at full frame rate
    engine.beginFrame(deltaTime);
    renderScene();
    engine.endFrame();
    
    // Throttle UI updates
    if ((now - lastUpdate) >= UPDATE_INTERVAL_NS) {
        if (engine.isTelemetryEnabled()) {
            updateTelemetryUI();
        }
        lastUpdate = now;
    }
}
```

## Example Application

See `com.astraeus.examples.TelemetryIntegrationExample` for a complete working example demonstrating:
- Viewport with overlay
- Side panel with detailed telemetry
- Throttled updates
- Toggle controls
- Proper integration patterns

## Native C API Mapping

The JavaFX UI components integrate with the following native C API functions (from `engine/api/EngineAPI.h`):

```c
// Enable/disable telemetry
void astraeus_enable_telemetry(EngineHandle engine, bool enabled);
bool astraeus_is_telemetry_enabled(EngineHandle engine);

// Get current frame stats
void astraeus_get_telemetry_frame_stats(EngineHandle engine, TelemetryFrameStats* out_stats);

// Get history (ring buffer)
uint32_t astraeus_get_telemetry_history(EngineHandle engine, TelemetryFrameStats* out_buffer, uint32_t max_frames);

// Get per-pass timings
uint32_t astraeus_get_pass_count(EngineHandle engine);
bool astraeus_get_pass_timing(EngineHandle engine, uint32_t pass_index, 
                               char* out_name_buffer, uint32_t name_buffer_size, 
                               double* out_time_ms);
```

## Building

Requires:
- Java 21+ (for FFM - Foreign Function & Memory API)
- JavaFX 21+
- Maven 3.6+

Build:
```bash
mvn compile
```

Note: The project uses FFM which requires Java 21+ runtime.

## Future Enhancements

Potential improvements:
1. **Live charts** - Add JavaFX LineChart for FPS/frame time history
2. **GPU timing** - Display actual GPU query times when available
3. **Memory stats** - Show native memory usage
4. **Export data** - Export telemetry history to CSV/JSON
5. **Configurable thresholds** - Highlight performance warnings (FPS < 30, etc.)
6. **Custom metrics** - Allow user-defined telemetry counters
