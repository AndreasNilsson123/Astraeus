# Telemetry UI Quick Reference Card

## Running the Demo

```bash
# Compile
mvn clean compile

# Run demo application
mvn javafx:run -Djavafx.mainClass=com.astraeus.test.TelemetryDemoApp
```

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **F3** | Toggle telemetry overlay (HUD) |
| **T** | Toggle telemetry panel (detailed table) |
| **E** | Toggle telemetry collection (enable/disable) |

## Components

### TelemetryOverlay (HUD)
```java
TelemetryOverlay overlay = new TelemetryOverlay();
overlay.setVisible(true);
```

Displays:
- FPS
- CPU time (ms)
- GPU time (ms)
- Draw calls
- Triangle count

### TelemetryPane (Table)
```java
TelemetryPane pane = new TelemetryPane(engine);
```

Shows:
- Per-pass breakdown
- Duration in milliseconds
- Percentage of total frame time

## Integration Example

```java
// Create reusable views (avoid allocations)
FrameStatsView frameStats = new FrameStatsView();

// Create overlay
TelemetryOverlay overlay = new TelemetryOverlay();
StackPane container = new StackPane();
container.getChildren().addAll(viewport, overlay);
StackPane.setAlignment(overlay, Pos.TOP_LEFT);

// Create pane
TelemetryPane pane = new TelemetryPane(engine);

// Update at 30 Hz in render loop
private long lastTelemetryUpdate = 0;
private static final long UPDATE_INTERVAL_NS = 33_000_000; // 30 Hz

// In AnimationTimer:
if (now - lastTelemetryUpdate >= UPDATE_INTERVAL_NS) {
    engine.getFrameStats(frameStats);
    overlay.update(frameStats);
    pane.update();
    lastTelemetryUpdate = now;
}
```

## Files Created

### Source Code
- `java/src/main/java/com/astraeus/tools/TelemetryOverlay.java`
- `java/src/main/java/com/astraeus/tools/TelemetryPane.java`
- `java/src/main/java/com/astraeus/test/TelemetryDemoApp.java`

### Documentation
- `TELEMETRY_UI_README.md` - Full user guide
- `TASK_C1_FINAL_SUMMARY.md` - Implementation summary
- `TELEMETRY_UI_VISUAL_GUIDE.txt` - Layout diagrams

## Performance Tips

1. **Use reusable views** - No per-frame allocations
2. **Update at 30 Hz** - Not every frame
3. **Disable when not needed** - Zero overhead
4. **Hide overlay/pane** - Saves update time

## Troubleshooting

### Overlay not visible?
- Check `overlay.setVisible(true)`
- Ensure positioned in StackPane
- Verify viewport is rendered

### Table empty?
- Check `engine.isTelemetryEnabled()`
- Call `pane.update()` in render loop
- Verify pass count > 0

### Performance impact?
- Should be <1% FPS when enabled
- 0% when disabled
- Update rate at 30 Hz, not 60+

## Quick Test

```bash
# Validate build
./validate_telemetry_ui.sh

# Should output:
# ✓ All validation checks passed!
```

## Support

See comprehensive documentation:
- `TELEMETRY_UI_README.md` - Full guide
- `TASK_C1_FINAL_SUMMARY.md` - Complete reference
