# Viewport Framework v2

Enhanced JavaFX viewport system with camera control, input routing, and overlay management.

## Overview

The Viewport Framework v2 provides a complete solution for building 3D editor-style viewports in JavaFX with:

- **Camera Control**: Three camera modes (Orbit, Fly, Pan) with smooth input handling
- **Input Routing**: Proper event isolation for multiple independent viewports
- **Overlay Stack**: Layered UI system for HUD, selections, gizmos, and other overlays
- **Performance**: Zero per-frame allocations in hot paths

## Components

### ViewportController

Handles camera movement and control for a viewport.

**Camera Modes:**
- **Orbit**: Rotate around a target point (left drag to rotate, middle drag to pan target, scroll to zoom)
- **Fly**: Free-flying FPS-style camera (WASD to move, mouse to look, QE for up/down)
- **Pan**: 2D panning mode (drag to pan, scroll to zoom)

**Usage:**
```java
ViewportController controller = new ViewportController(engine);
controller.setMode(ViewportController.Mode.ORBIT);

// In event handlers:
viewport.setOnMousePressed(controller::handleMousePressed);
viewport.setOnMouseDragged(controller::handleMouseDragged);
viewport.setOnMouseReleased(controller::handleMouseReleased);
viewport.setOnScroll(controller::handleScroll);
viewport.setOnKeyPressed(controller::handleKeyPressed);
viewport.setOnKeyReleased(controller::handleKeyReleased);

// In update loop:
controller.update(deltaTime);

// Get camera state:
double[] position = controller.getCameraPosition();
double[] target = controller.getCameraTarget();
```

**Keyboard Shortcuts:**
- `1` - Switch to Orbit mode
- `2` - Switch to Fly mode  
- `3` - Switch to Pan mode
- `WASD` - Move camera (Fly mode)
- `QE` - Move up/down (Fly mode)
- `Shift` - Sprint modifier (Fly mode)

### OverlayStack

Manages layered UI overlays with Z-order and visibility control.

**Layer Types:**
- `BACKGROUND` (0) - Behind viewport image
- `SELECTION` (1) - Selection indicators
- `GIZMO` (2) - 3D manipulation widgets
- `HUD` (3) - Text overlays and telemetry

**Usage:**
```java
OverlayStack overlayStack = new OverlayStack();

// Add overlays to specific layers
TelemetryOverlay telemetry = new TelemetryOverlay();
overlayStack.addOverlay("telemetry", telemetry, OverlayStack.Layer.HUD);

Rectangle selection = new Rectangle();
overlayStack.addOverlay("selection", selection, OverlayStack.Layer.SELECTION);

// Control visibility
overlayStack.setOverlayVisible("telemetry", true);
overlayStack.toggleOverlay("selection");

// Get overlay
Node overlay = overlayStack.getOverlay("telemetry");
```

### FxViewportV2

Enhanced viewport component that integrates camera control, overlays, and input routing.

**Features:**
- Built-in camera controller integration
- Overlay stack management
- Entity picking support
- Safe pixel buffer handling
- Input event routing
- Focus management

**Usage:**
```java
FxViewportV2 viewport = new FxViewportV2(engine, 2560, 1440, 1280, 720);

// Configure camera
viewport.getController().setMode(ViewportController.Mode.ORBIT);

// Setup picking callback
viewport.setOnEntitySelected(result -> {
    if (result.hasValidEntity()) {
        System.out.println("Selected entity: " + result.getEntityId());
    }
});

// Add custom overlay
Label customOverlay = new Label("Custom HUD");
viewport.getOverlayStack().addOverlay("custom", customOverlay, 
                                      OverlayStack.Layer.HUD, 
                                      Pos.BOTTOM_RIGHT);

// In render loop:
viewport.update(deltaTime);
viewport.updateDisplay();
```

**Built-in Overlays:**
- `"selection"` - Yellow selection rectangle (Layer: SELECTION)
- `"camera-info"` - Camera position/mode info (Layer: HUD)
- `"telemetry"` - FPS and performance stats (Layer: HUD)

**Keyboard Shortcuts:**
- `F1` - Toggle camera info overlay
- `F2` - Toggle telemetry overlay
- `ESC` - Clear selection
- `1/2/3` - Camera mode switching
- Camera-specific keys (see ViewportController)

## Multi-Viewport Support

The framework fully supports multiple independent viewports with proper input isolation.

**Example:**
```java
// Create two separate engines and viewports
NativeEngine engine1 = new NativeEngine(1280, 720, true);
NativeEngine engine2 = new NativeEngine(1280, 720, true);

FxViewportV2 viewport1 = new FxViewportV2(engine1, 2560, 1440, 800, 600);
FxViewportV2 viewport2 = new FxViewportV2(engine2, 2560, 1440, 800, 600);

// Each viewport has independent camera control
viewport1.getController().setMode(ViewportController.Mode.ORBIT);
viewport2.getController().setMode(ViewportController.Mode.FLY);

// Layout in split pane
SplitPane splitPane = new SplitPane(viewport1, viewport2);

// Track focus for active viewport
viewport1.focusedProperty().addListener((obs, was, is) -> {
    if (is) activeViewport = viewport1;
});
viewport2.focusedProperty().addListener((obs, was, is) -> {
    if (is) activeViewport = viewport2;
});
```

See `MultiViewportDemo.java` for a complete working example.

## Performance Considerations

### Zero Per-Frame Allocations

All viewport components are designed to avoid per-frame allocations:

- **ViewportController**: Reuses internal state variables for input tracking
- **OverlayStack**: Fixed layer structure created once at construction
- **FxViewportV2**: No temporary objects in update/render loop

### Efficient Input Handling

- Events are consumed to prevent propagation
- Focus tracking ensures only active viewport processes input
- Mouse coordinate calculations use pre-allocated variables

### Safe Memory Management

- PixelBuffer backing memory allocated once at maximum size
- Viewport resizing only changes the view region, not buffer
- No memory reallocation during runtime

## Demo Application

`MultiViewportDemo.java` demonstrates all features:

```bash
cd /path/to/Astraeus
mvn javafx:run -pl java
```

Or run directly:
```bash
java -cp target/classes com.astraeus.test.MultiViewportDemo.Starter
```

**Features Demonstrated:**
- Two independent viewports side-by-side
- Separate camera controllers (Orbit vs Fly)
- Focus tracking with visual feedback
- Toolbar controls for all features
- Real-time FPS display per viewport
- Entity picking and selection
- Overlay visibility toggles

## Migration from FxViewport v1

The original `FxViewport` class remains unchanged for backward compatibility. To migrate to v2:

1. Replace `FxViewport` with `FxViewportV2`
2. Remove manual input handling - v2 has it built-in
3. Use integrated overlay stack instead of manual overlay management
4. Use controller methods for camera state instead of external management

**Before (v1):**
```java
FxViewport viewport = new FxViewport(engine, maxW, maxH, w, h);

// Manual input handling
viewport.setOnMousePressed(e -> handleMouse(e));

// Manual overlay management
Rectangle overlay = new Rectangle();
viewport.getChildren().add(overlay);
```

**After (v2):**
```java
FxViewportV2 viewport = new FxViewportV2(engine, maxW, maxH, w, h);

// Built-in input routing - no manual handling needed
// Camera controller automatically handles input

// Use overlay stack
viewport.getOverlayStack().addOverlay("my-overlay", overlay, 
                                      OverlayStack.Layer.SELECTION);
```

## Future Enhancements

Potential additions for future versions:

- [ ] Camera animation/transitions between modes
- [ ] Configurable input bindings
- [ ] Touch/gesture support
- [ ] Multi-touch camera control
- [ ] Camera state serialization/loading
- [ ] Gizmo system implementation
- [ ] Viewport snapping and guides
- [ ] Grid overlay support
- [ ] Viewport cloning/mirroring

## Architecture Notes

### Why Separate Components?

The framework is split into three components for flexibility:

- **ViewportController**: Can be used standalone or with any rendering system
- **OverlayStack**: Generic overlay manager, not viewport-specific
- **FxViewportV2**: Integrates everything for turnkey usage

This allows mixing and matching components as needed.

### Thread Safety

All components are designed for JavaFX Application Thread only. Do not call methods from background threads. Use `Platform.runLater()` if needed.

### Memory Safety

The framework follows the engine's memory safety contract:
- Native memory is allocated once and never resized
- JavaFX PixelBuffer wraps stable native memory
- No dangling pointers or buffer invalidation possible

## License

Part of the Astraeus visualization engine. See main LICENSE file.
