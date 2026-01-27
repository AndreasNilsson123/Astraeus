# Telemetry UI Components - Task C1

This document describes the JavaFX UI components for the Astraeus telemetry system.

## Overview

The telemetry system provides real-time performance monitoring for the Astraeus 3D visualization engine. It consists of two main UI components:

1. **TelemetryOverlay** - A compact HUD-style overlay for real-time stats
2. **TelemetryPane** - A detailed panel with per-pass performance breakdown

## Components

### 1. TelemetryOverlay.java

A lightweight HUD overlay that displays real-time performance metrics over the viewport.

**Location:** `java/src/main/java/com/astraeus/tools/TelemetryOverlay.java`

**Features:**
- FPS (frames per second)
- CPU frame time (render_time_ms)
- GPU frame time (gpu_time_ms)
- Draw calls count
- Triangle count (formatted with K/M suffixes)
- Semi-transparent dark background with white text
- Minimal allocations per frame
- Can be positioned in any corner

**Usage:**
```java
// Create overlay
TelemetryOverlay overlay = new TelemetryOverlay();
overlay.setVisible(true);

// Position in corner of viewport (using StackPane)
StackPane viewportContainer = new StackPane();
viewportContainer.getChildren().addAll(viewport, overlay);
StackPane.setAlignment(overlay, Pos.TOP_LEFT);
StackPane.setMargin(overlay, new Insets(10));

// Update each frame (or at controlled rate)
engine.getFrameStats(frameStats);
overlay.update(frameStats);
```

**Example Display:**
```
┌──────────────────────┐
│ FPS: 60.0            │
│ CPU: 12.5ms          │
│ GPU: 8.3ms           │
│ Draws: 125           │
│ Tris: 45.2K          │
└──────────────────────┘
```

### 2. TelemetryPane.java

A detailed panel showing per-pass performance breakdown in a table.

**Location:** `java/src/main/java/com/astraeus/tools/TelemetryPane.java`

**Features:**
- TableView with pass-by-pass breakdown
- Columns: Pass Name, Duration (ms), Percentage (%)
- Summary header showing total passes and frame time
- Enable/disable telemetry checkbox
- Auto-refresh when telemetry is enabled
- Reusable views to avoid allocations

**Usage:**
```java
// Create pane
TelemetryPane pane = new TelemetryPane(engine);

// Add to UI (e.g., in a tab or side panel)
TabPane tabPane = new TabPane();
Tab telemetryTab = new Tab("Telemetry", pane);
tabPane.getTabs().add(telemetryTab);

// Update at controlled rate (e.g., 30 Hz)
if (engine.isTelemetryEnabled()) {
    pane.update();
}
```

**Example Table:**
```
Pass Name       | Duration | %
----------------|----------|------
ClearPass       | 0.5ms    | 3%
GridPass        | 2.1ms    | 15%
AxesPass        | 0.8ms    | 6%
PointSpritePass | 8.2ms    | 58%
TrailPass       | 1.9ms    | 14%
TrianglePass    | 0.6ms    | 4%
----------------|----------|------
Total           | 13.9ms   | 72 FPS
```

## Demo Application

### TelemetryDemoApp.java

A complete demonstration application showcasing all telemetry features.

**Location:** `java/src/main/java/com/astraeus/test/TelemetryDemoApp.java`

**Features:**
- Full integration of TelemetryOverlay and TelemetryPane
- Keyboard shortcuts for quick access
- Controlled update rate (30 Hz) to avoid UI churn
- Integration with existing picking and scene inspector

**Keyboard Shortcuts:**
- **F3** - Toggle telemetry overlay
- **T** - Toggle telemetry panel visibility
- **E** - Toggle telemetry collection (enable/disable)

**Running the Demo:**
```bash
# Build the project
mvn clean package

# Run the telemetry demo
mvn javafx:run -Djavafx.mainClass=com.astraeus.test.TelemetryDemoApp
```

## API Reference

### NativeEngine Telemetry Methods

The following methods are available on the `NativeEngine` class:

```java
// Enable/disable telemetry collection
void setTelemetryEnabled(boolean enabled);
boolean isTelemetryEnabled();

// Get frame statistics
void getFrameStats(FrameStatsView statsView);

// Get per-pass telemetry
int getPassCount();
boolean getPassTelemetry(int passIndex, PassTelemetryView telemetryView);
```

### FrameStatsView

Reusable view for frame statistics (avoid per-frame allocations):

```java
long getFrameNumber();
double getDeltaTimeMs();
double getRenderTimeMs();
double getGpuTimeMs();
int getDrawCalls();
int getTriangleCount();
int getEntityCount();
double getFPS();  // calculated from delta time
```

### PassTelemetryView

Reusable view for per-pass telemetry:

```java
String getPassName();
double getDurationMs();
double getPercentage(double totalFrameTimeMs);
```

## Performance Considerations

### Controlled Update Rate

To avoid UI churn and unnecessary overhead, telemetry UI should be updated at a controlled rate (e.g., 30 Hz) rather than every frame:

```java
private long lastTelemetryUpdateTime;
private static final long TELEMETRY_UPDATE_INTERVAL_NS = 33_000_000; // ~30 Hz

// In render loop:
if (now - lastTelemetryUpdateTime >= TELEMETRY_UPDATE_INTERVAL_NS) {
    updateTelemetry();
    lastTelemetryUpdateTime = now;
}
```

### Reusable Views

Always reuse `FrameStatsView` and `PassTelemetryView` instances:

```java
// Create once (as instance variables)
private final FrameStatsView frameStats = new FrameStatsView();
private final PassTelemetryView passTelemetry = new PassTelemetryView();

// Reuse every frame
engine.getFrameStats(frameStats);
overlay.update(frameStats);
```

### Conditional Updates

Only update UI components when visible:

```java
if (telemetryOverlay.isVisible()) {
    telemetryOverlay.update(frameStats);
}

if (telemetryPane.isVisible()) {
    telemetryPane.update();
}
```

## Integration Guidelines

### Adding to Existing Applications

To add telemetry to an existing application:

1. **Create the overlay:**
   ```java
   TelemetryOverlay overlay = new TelemetryOverlay();
   ```

2. **Position over viewport:**
   ```java
   StackPane container = new StackPane();
   container.getChildren().addAll(viewport, overlay);
   StackPane.setAlignment(overlay, Pos.TOP_LEFT);
   StackPane.setMargin(overlay, new Insets(10));
   ```

3. **Create telemetry pane:**
   ```java
   TelemetryPane pane = new TelemetryPane(engine);
   // Add to tab or side panel
   ```

4. **Add keyboard shortcuts:**
   ```java
   scene.addEventHandler(KeyEvent.KEY_PRESSED, event -> {
       if (event.getCode() == KeyCode.F3) {
           overlay.setVisible(!overlay.isVisible());
           event.consume();
       }
   });
   ```

5. **Update in render loop:**
   ```java
   // At controlled rate (e.g., 30 Hz)
   engine.getFrameStats(frameStats);
   overlay.update(frameStats);
   pane.update();
   ```

## Styling Customization

### Overlay Styling

The overlay uses CSS for styling. You can customize:

```java
overlay.setStyle(
    "-fx-background-color: rgba(0, 0, 0, 0.75); " +
    "-fx-background-radius: 5; " +
    "-fx-border-color: rgba(255, 255, 255, 0.2); " +
    "-fx-border-width: 1;"
);
```

### Table Styling

The telemetry pane table can be styled via CSS:

```css
.table-view {
    -fx-background-color: white;
}

.table-cell {
    -fx-font-family: monospace;
}
```

## Testing

### Manual Testing

1. Build the project:
   ```bash
   mvn clean package
   ```

2. Run the telemetry demo:
   ```bash
   mvn javafx:run -Djavafx.mainClass=com.astraeus.test.TelemetryDemoApp
   ```

3. Verify:
   - Overlay displays FPS, CPU/GPU time, draw calls, triangles
   - F3 toggles overlay visibility
   - T toggles telemetry panel
   - E toggles telemetry collection
   - Table shows per-pass breakdown
   - Values update in real-time

### Performance Testing

1. Enable telemetry and verify overhead is minimal (<1% FPS impact)
2. Test with telemetry disabled (should have zero overhead)
3. Test controlled update rate (30 Hz) vs per-frame updates
4. Verify no allocations per frame (use profiler)

## Known Limitations

1. **Telemetry overhead:** When enabled, telemetry adds minor overhead (~1-2%) due to GPU timer queries
2. **Update rate:** UI updates at 30 Hz to avoid churn; not every frame is displayed
3. **Table size:** Large numbers of passes may require scrolling in the table

## Future Enhancements

Potential improvements for future versions:

1. **Graph visualization:** Line charts for FPS/frame time history
2. **Export capabilities:** Save telemetry data to CSV/JSON
3. **Threshold alerts:** Visual warnings when FPS drops below threshold
4. **Comparison mode:** Compare telemetry across multiple runs
5. **Custom metrics:** User-defined telemetry counters
6. **Heatmap visualization:** Per-pass performance heatmap

## Architecture Notes

### Design Principles

1. **Minimal allocations:** Reuse view objects to avoid per-frame GC pressure
2. **Controlled updates:** Update UI at fixed rate (30 Hz) to avoid churn
3. **Separation of concerns:** Overlay for quick glance, pane for detailed analysis
4. **Zero overhead when disabled:** Telemetry can be completely disabled
5. **Stable ABI:** Native API uses simple structs, no callbacks into Java

### Thread Safety

- All UI updates happen on JavaFX Application Thread
- Native engine calls are thread-safe
- Reusable views are not thread-safe (use one per thread)

## Files Created

- `java/src/main/java/com/astraeus/tools/TelemetryOverlay.java` - HUD overlay component
- `java/src/main/java/com/astraeus/tools/TelemetryPane.java` - Detailed telemetry panel
- `java/src/main/java/com/astraeus/test/TelemetryDemoApp.java` - Demo application
- `TELEMETRY_UI_README.md` - This documentation

## Related Documentation

- `TASK_C1_COMPLETION_REPORT.md` - Native telemetry implementation details
- `TASK_C1_SUMMARY.md` - Overall telemetry system design
- `java/src/main/java/com/astraeus/native_api/NativeEngine.java` - Engine API
- `java/src/main/java/com/astraeus/native_api/FrameStatsView.java` - Frame stats view
- `java/src/main/java/com/astraeus/native_api/PassTelemetryView.java` - Pass telemetry view

## Support

For questions or issues, please refer to the main project documentation or contact the development team.
