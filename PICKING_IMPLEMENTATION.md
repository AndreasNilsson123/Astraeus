# Task B2: Picking via ID Buffer - Implementation Guide

## Overview

This implementation adds entity picking functionality to the Astraeus 3D visualization engine. Users can click on entities in the JavaFX viewport to select them, view metadata, and see visual feedback.

## Architecture

### C++ Layer (Already Implemented)
The C++ engine already provides:
- **ID Buffer Rendering**: R32UI texture containing entity IDs
- **`astraeus_pick(engine, screen_x, screen_y)`**: Returns `PickResult` struct
- **`astraeus_get_id_buffer()`**: Returns `PixelBufferView` for ID buffer readback

### Java FFM Bindings Layer

#### EngineBindings.java
Added FFM bindings for the picking API:
```java
// PickResult struct layout (24 bytes)
public static final StructLayout PICK_RESULT_LAYOUT = MemoryLayout.structLayout(
    ValueLayout.JAVA_INT.withName("entity_id"),      // 4 bytes
    ValueLayout.JAVA_FLOAT.withName("depth"),        // 4 bytes
    ValueLayout.JAVA_FLOAT.withName("world_x"),      // 4 bytes
    ValueLayout.JAVA_FLOAT.withName("world_y"),      // 4 bytes
    ValueLayout.JAVA_FLOAT.withName("world_z"),      // 4 bytes
    ValueLayout.JAVA_BOOLEAN.withName("hit"),        // 1 byte
    MemoryLayout.paddingLayout(3)                    // 3 bytes padding
);

// Function descriptor
private static final FunctionDescriptor PICK_DESC = FunctionDescriptor.of(
    PICK_RESULT_LAYOUT,   // return: PickResult (struct by value)
    ValueLayout.ADDRESS,  // param: EngineHandle
    ValueLayout.JAVA_INT, // param: screen_x
    ValueLayout.JAVA_INT  // param: screen_y
);

// Method handle
public static final MethodHandle PICK;
```

### Java Wrapper Layer

#### PickingView.java
Immutable wrapper for `PickResult` struct:
```java
public class PickingView {
    private final int entityId;
    private final float depth;
    private final float worldX, worldY, worldZ;
    private final boolean hit;
    
    public PickingView(MemorySegment structSegment) {
        // Safe extraction using VarHandles
    }
    
    public boolean hasValidEntity() {
        return hit && entityId != 0;
    }
}
```

#### NativeEngine.java
High-level API for picking:
```java
public PickingView pick(int screenX, int screenY) {
    MemorySegment resultStruct = (MemorySegment) EngineBindings.PICK.invoke(
        engineHandle, screenX, screenY);
    return new PickingView(resultStruct);
}
```

### JavaFX UI Layer

#### FxViewport.java
Interactive viewport with picking support:
```java
// Mouse click handler
private void handleMouseClick(MouseEvent event) {
    // 1. Convert scene coordinates to viewport coordinates
    double scaleX = currentWidth / imageView.getFitWidth();
    double scaleY = currentHeight / imageView.getFitHeight();
    int viewportX = (int) (event.getX() * scaleX);
    int viewportY = (int) (event.getY() * scaleY);
    
    // 2. Perform picking
    PickingView result = engine.pick(viewportX, viewportY);
    
    // 3. Update selection state and visual feedback
    if (result.hasValidEntity()) {
        selectedEntityId = result.getEntityId();
        updateSelectionOverlay(event.getX(), event.getY());
        
        // Notify callback
        if (onEntitySelected != null) {
            onEntitySelected.accept(result);
        }
    }
}

// Visual feedback: yellow outline rectangle
private Rectangle selectionOverlay;
```

Features:
- **Coordinate transformation**: Scene coords → Viewport coords
- **Selection overlay**: 40x40px yellow rectangle at click location
- **Callback support**: `setOnEntitySelected(Consumer<PickingView>)`
- **Selection state management**: `getSelectedEntityId()`, `clearSelection()`

#### SceneInspector.java
Metadata panel for selected entities:
```java
public class SceneInspector extends VBox {
    public void updateSelection(PickingView pickResult) {
        // Display entity ID, world position, depth
        // Show entity metadata (transform, rendering properties)
    }
}
```

Displays:
- Entity ID
- World position (x, y, z)
- Depth value
- Transform properties
- Rendering properties

#### PickingDemoApp.java
Complete demonstration application showing:
- Entity creation
- Click-to-select functionality
- Selection highlighting
- Metadata display
- Viewport resizing (picking still works)
- Clear selection

## Usage Example

```java
// Create engine and viewport
NativeEngine engine = new NativeEngine(1280, 720, true);
FxViewport viewport = new FxViewport(engine, 2560, 1440, 1280, 720);

// Create inspector
SceneInspector inspector = new SceneInspector();

// Setup picking callback
viewport.setOnEntitySelected(pickResult -> {
    inspector.updateSelection(pickResult);
    System.out.println("Selected: " + pickResult);
});

// Add viewport to scene
scene.setCenter(viewport);
scene.setRight(inspector);

// Render loop
AnimationTimer timer = new AnimationTimer() {
    public void handle(long now) {
        engine.beginFrame(deltaTime);
        engine.endFrame();
        viewport.updateDisplay();
    }
};
timer.start();
```

## Building and Running

### Prerequisites
1. **Java 21+** or **Java 17+ with preview features**
   - FFM (Foreign Function & Memory API) required
   - Update `pom.xml` if using Java 21+

2. **C++ Native Library**
   - Build with CMake: `./build.sh`
   - Ensure `libastraeus.so` (Linux), `astraeus.dll` (Windows), or `libastraeus.dylib` (macOS) is in library path

### Compilation

```bash
# Compile Java code
mvn clean compile

# Run demo application
mvn javafx:run -Djavafx.mainClass=com.astraeus.test.PickingDemoApp
```

### Native Library Setup

Ensure the native library is accessible:
```bash
# Linux
export LD_LIBRARY_PATH=/path/to/astraeus/build:$LD_LIBRARY_PATH

# macOS
export DYLD_LIBRARY_PATH=/path/to/astraeus/build:$DYLD_LIBRARY_PATH

# Windows
set PATH=C:\path\to\astraeus\build;%PATH%
```

## Acceptance Criteria

✅ **Clicking entity selects it reliably**
- Mouse click handler with proper coordinate transformation
- Direct invocation of `engine.pick(x, y)`

✅ **Visual feedback**
- Yellow selection overlay rectangle (40x40px)
- Positioned at click location

✅ **Metadata display**
- Entity ID, world position, depth shown in inspector
- Metadata panel updates on selection

✅ **Works under resizing**
- Coordinate transformation accounts for viewport scale
- Picking remains accurate after resize

✅ **Works with camera movement**
- C++ pick implementation uses current camera matrices
- World position correctly calculated

## Implementation Details

### Coordinate Systems

1. **Scene Coordinates**: JavaFX scene space (e.g., 0-1600)
2. **ImageView Coordinates**: May be scaled/stretched
3. **Viewport Coordinates**: Native engine space (e.g., 0-1280)

Transformation:
```java
double scaleX = currentWidth / imageView.getFitWidth();
double scaleY = currentHeight / imageView.getFitHeight();
int viewportX = (int) (sceneX * scaleX);
int viewportY = (int) (sceneY * scaleY);
```

### Memory Safety

- **PickingView is immutable**: Values copied from MemorySegment on construction
- **No dangling pointers**: Result struct is passed by value from C++
- **Arena-managed memory**: All native allocations use Arena lifecycle

### Performance Considerations

- **Picking is synchronous**: Blocks until result returned
- **No per-frame allocation**: PickingView creation on-demand only
- **ID buffer readback**: Already optimized in C++ layer with double-buffering

## Testing

### Manual Testing
1. Run `PickingDemoApp`
2. Click "Create Entity" to add entities
3. Click entities in viewport
4. Verify:
   - Yellow outline appears
   - Inspector shows correct entity ID and position
   - Selection persists across frames
   - Clear selection works

### Resize Testing
1. Start at 1280x720
2. Click entity and note selection
3. Resize to 800x600
4. Click same entity - should still work
5. Resize to 1920x1080
6. Picking should remain accurate

### Camera Movement Testing
1. Select an entity
2. Move camera (if camera controls implemented)
3. Re-select same entity
4. Verify world position updates correctly

## Future Enhancements

1. **Multi-selection**: Ctrl+Click to select multiple entities
2. **Drag selection**: Rectangle selection region
3. **Hover preview**: Show entity info on hover
4. **Selection highlighting in 3D**: Modify entity color in renderer
5. **Picking filters**: Allow/deny picking by entity type
6. **Ray-cast picking**: Alternative to ID buffer for higher precision

## Troubleshooting

### "Failed to load Astraeus native library"
- Build C++ library with CMake
- Add library to system path
- Check library naming: `libastraeus.so` (Linux), `astraeus.dll` (Windows)

### "package ValueLayout does not exist"
- Update to Java 21+ or enable preview features in Java 17+
- Verify `--enable-preview` in `pom.xml`

### Picking returns no hit
- Verify ID buffer is configured: `engine.configureReadback()`
- Check entities are visible and renderable
- Ensure viewport coordinates are within bounds

### Selection overlay not visible
- Check overlay is added to scene graph: `getChildren().add(selectionOverlay)`
- Verify overlay is not behind viewport: Add after imageView
- Ensure `setVisible(true)` is called

## References

- **C API**: `engine/api/EngineAPI.h` - `astraeus_pick()`, `PickResult`
- **FFM Bindings**: `java/src/main/java/com/astraeus/native_api/EngineBindings.java`
- **Wrapper Layer**: `java/src/main/java/com/astraeus/native_api/NativeEngine.java`
- **UI Integration**: `java/src/main/java/com/astraeus/rendering/FxViewport.java`
- **Demo App**: `java/src/main/java/com/astraeus/test/PickingDemoApp.java`
