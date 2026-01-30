# Viewport Framework v2 Architecture

## Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      FxViewportV2                            │
│  ┌────────────────────────────────────────────────────────┐ │
│  │              ImageView (Engine Output)                  │ │
│  └────────────────────────────────────────────────────────┘ │
│                            │                                 │
│                            │ PixelBuffer                     │
│                            │                                 │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                   OverlayStack                          │ │
│  │  ┌──────────────────────────────────────────────────┐  │ │
│  │  │  Layer 3 (HUD)                                    │  │ │
│  │  │  - Camera Info Label                              │  │ │
│  │  │  - Telemetry Overlay                              │  │ │
│  │  └──────────────────────────────────────────────────┘  │ │
│  │  ┌──────────────────────────────────────────────────┐  │ │
│  │  │  Layer 2 (GIZMO)                                  │  │ │
│  │  │  - (Reserved for future gizmo widgets)            │  │ │
│  │  └──────────────────────────────────────────────────┘  │ │
│  │  ┌──────────────────────────────────────────────────┐  │ │
│  │  │  Layer 1 (SELECTION)                              │  │ │
│  │  │  - Selection Rectangle                            │  │ │
│  │  └──────────────────────────────────────────────────┘  │ │
│  │  ┌──────────────────────────────────────────────────┐  │ │
│  │  │  Layer 0 (BACKGROUND)                             │  │ │
│  │  │  - (Reserved for background overlays)             │  │ │
│  │  └──────────────────────────────────────────────────┘  │ │
│  └────────────────────────────────────────────────────────┘ │
│                            │                                 │
│  ┌────────────────────────────────────────────────────────┐ │
│  │              ViewportController                         │ │
│  │                                                         │ │
│  │  Modes:                                                 │ │
│  │  - ORBIT (rotate around target)                        │ │
│  │  - FLY   (free camera movement)                        │ │
│  │  - PAN   (2D panning)                                  │ │
│  │                                                         │ │
│  │  Input:                                                 │ │
│  │  - Mouse: Press/Drag/Release/Scroll                    │ │
│  │  - Keyboard: WASD, QE, 1/2/3                           │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                           │
                           ▼
                  ┌────────────────┐
                  │ NativeEngine   │
                  │ (C++ Core)     │
                  └────────────────┘
```

## Data Flow

### Input Flow
```
User Input (Mouse/Keyboard)
         │
         ▼
   FxViewportV2
    (Input Routing)
         │
         ├──────────────┐
         │              │
         ▼              ▼
ViewportController   Picking
    (Camera)         (Entity)
         │              │
         ▼              ▼
    Camera Pos     Selection State
    Camera Target  Selection Overlay
```

### Render Flow
```
Engine Render
      │
      ▼
PixelBuffer Update
      │
      ▼
ImageView Display
      │
      ▼
OverlayStack Render
      │
      ├─── HUD Overlays (text, telemetry)
      ├─── Gizmo Layer (future)
      ├─── Selection Indicators
      └─── Background Effects
```

## Class Relationships

```
┌──────────────────┐
│ ViewportController│◄──────┐
│                  │        │
│ + Mode mode      │        │ composition
│ + update()       │        │
│ + handleInput()  │        │
└──────────────────┘        │
                            │
┌──────────────────┐        │
│  OverlayStack    │◄───────┼────────┐
│                  │        │        │
│ + Layer enum     │        │        │
│ + addOverlay()   │        │        │
│ + toggleOverlay()│        │        │
└──────────────────┘        │        │
                            │        │
                   ┌────────┴───────┐│
                   │  FxViewportV2  ││
                   │                ││
                   │ - controller   ││
                   │ - overlayStack ││
                   │ - engine       ││
                   │                ││
                   │ + update()     ││
                   │ + updateDisplay()│
                   └─────────────────┘
                            │
                            │ uses
                            ▼
                   ┌─────────────────┐
                   │ NativeEngine    │
                   │                 │
                   │ + beginFrame()  │
                   │ + endFrame()    │
                   │ + pick()        │
                   └─────────────────┘
```

## Multi-Viewport Architecture

```
┌──────────────────────────────────────────────────┐
│              Application Window                   │
│                                                   │
│  ┌────────────────────┬────────────────────┐    │
│  │   FxViewportV2 #1  │  FxViewportV2 #2   │    │
│  │                    │                    │    │
│  │  ┌──────────────┐  │  ┌──────────────┐  │    │
│  │  │ Controller#1 │  │  │ Controller#2 │  │    │
│  │  │ Mode: ORBIT  │  │  │ Mode: FLY    │  │    │
│  │  └──────────────┘  │  └──────────────┘  │    │
│  │  ┌──────────────┐  │  ┌──────────────┐  │    │
│  │  │ OverlayStack │  │  │ OverlayStack │  │    │
│  │  └──────────────┘  │  └──────────────┘  │    │
│  │         │          │         │          │    │
│  │         ▼          │         ▼          │    │
│  │  ┌──────────────┐  │  ┌──────────────┐  │    │
│  │  │  Engine #1   │  │  │  Engine #2   │  │    │
│  │  └──────────────┘  │  └──────────────┘  │    │
│  └────────────────────┴────────────────────┘    │
│                                                   │
│  Focus Tracking: Yellow border on active viewport│
│  Input Isolation: Events consumed per viewport   │
└──────────────────────────────────────────────────┘
```

## Event Processing

### Mouse Event Flow
```
1. User clicks viewport
   ↓
2. FxViewportV2.setOnMousePressed()
   ↓
3. requestFocus() - ensure keyboard events
   ↓
4. Check inputEnabled flag
   ↓
5. controller.handleMousePressed(event)
   ↓
6. event.consume() - prevent propagation
   ↓
7. Controller updates internal state (isDragging, lastMouseX/Y)
```

### Camera Update Flow
```
1. AnimationTimer.handle()
   ↓
2. Calculate deltaTime
   ↓
3. viewport.update(deltaTime)
   ↓
4. controller.update(deltaTime)
   ↓
5. Based on mode:
   - ORBIT: (mouse-driven, no update needed)
   - FLY: Apply WASD movement
   - PAN: (mouse-driven, no update needed)
   ↓
6. Camera position/target updated
   ↓
7. Available via getCameraPosition/Target()
```

### Overlay Update Flow
```
1. viewport.update(deltaTime)
   ↓
2. Check if camera-info visible
   ↓
3. updateCameraInfo()
   - Get controller.getDebugInfo()
   - Update label text
   ↓
4. Check if telemetry visible
   ↓
5. Get engine.getTelemetryStats()
   ↓
6. telemetryOverlay.update(stats)
   - Update FPS, CPU, GPU labels
   - NO allocations (reuses Label instances)
```

## Performance Characteristics

### Memory Allocation Profile

**Startup (one-time allocations):**
- ViewportController: ~400 bytes (state variables)
- OverlayStack: ~1KB (layer containers, maps)
- FxViewportV2: ~2KB (UI components)

**Per-frame (hot path):**
- update(): 0 bytes ✅
- updateDisplay(): 0 bytes ✅
- handleInput(): 0 bytes ✅

**UI updates (infrequent):**
- String.format() in overlays: ~100 bytes at 30-60Hz
- Acceptable for UI updates

### CPU Profile (estimated)

**Per frame:**
- Input handling: <0.1ms
- Controller update: <0.1ms
- Overlay updates: <0.5ms (if visible)
- **Total overhead: <1ms** ✅

## Thread Safety

All components are **JavaFX Application Thread only**:
- No synchronization needed
- No concurrent access
- All UI updates via Platform.runLater() if needed

## Extension Points

### Adding New Camera Modes
```java
public enum Mode {
    ORBIT, FLY, PAN,
    TRACKBALL, // Add new mode
}

// In update():
switch (mode) {
    case TRACKBALL:
        updateTrackballCamera(deltaTime);
        break;
}
```

### Adding New Overlay Layers
```java
public enum Layer {
    BACKGROUND(0),
    SELECTION(1),
    GIZMO(2),
    HUD(3),
    DEBUG(4); // Add new layer
}
```

### Custom Overlays
```java
// Create custom overlay
VBox customHUD = new VBox();
customHUD.getChildren().addAll(
    new Label("Custom"),
    new Button("Action")
);

// Add to viewport
viewport.getOverlayStack()
    .addOverlay("custom", customHUD, Layer.HUD, Pos.TOP_LEFT);
```

## Design Decisions

### Why Three Separate Components?

**Flexibility:**
- Use ViewportController standalone with any renderer
- Use OverlayStack with any JavaFX container
- Use FxViewportV2 for turnkey integration

**Testability:**
- Each component can be tested independently
- ViewportController has no JavaFX dependencies (mock-friendly)

**Reusability:**
- Components can be mixed and matched
- Easy to create custom viewport variations

### Why Enum for Layers?

**Type Safety:**
- Compile-time checking of layer names
- No string typos

**Performance:**
- Enum comparisons are fast
- Fixed set of layers (no dynamic growth)

**Clarity:**
- Clear Z-order hierarchy
- Self-documenting code

### Why No Native Camera Integration Yet?

**Separation of Concerns:**
- Java side manages UI/input
- Native side manages rendering
- Camera state is computed in Java, can be passed to native later

**Flexibility:**
- Easy to switch between Java-side and native-side cameras
- Current design works with or without native camera support

## Future Improvements

### Camera Smoothing
```java
// Add smooth interpolation
private double smoothing = 0.1;
private double[] targetPos = new double[3];

public void update(double dt) {
    // Interpolate towards target
    currentPos[0] += (targetPos[0] - currentPos[0]) * smoothing;
    // ...
}
```

### Input Rebinding
```java
public class InputBindings {
    private Map<KeyCode, CameraAction> keyMap;
    
    public void bind(KeyCode key, CameraAction action) {
        keyMap.put(key, action);
    }
}
```

### Viewport Serialization
```java
public class ViewportState {
    public Mode cameraMode;
    public double[] position;
    public double[] target;
    
    public String toJSON() { /* ... */ }
    public static ViewportState fromJSON(String json) { /* ... */ }
}
```
