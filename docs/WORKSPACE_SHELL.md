# Astraeus Workspace Shell

## Overview

Professional JavaFX workspace with docking-like layout for the Astraeus visualization engine.

## Architecture

```
WorkspaceWindow (Main Container)
├── Menu Bar (File, View, Help)
├── Toolbar (Engine controls)
├── Main Workspace
│   ├── Left Panel: Scene Inspector
│   ├── Center: Tabbed Viewport Area
│   ├── Right Panel: Telemetry/Properties
│   └── Bottom: Console/Log Panel
└── Status Bar (Engine status, FPS, Memory)
```

## Components

### 1. WorkspaceWindow.java
Main workspace container with professional layout:
- **Split Panes**: Resizable vertical and horizontal splits
- **Menu System**: View menu for toggling panes
- **Layout Persistence**: Saves/restores divider positions and visibility
- **Status Bar**: Real-time engine status, FPS, and memory usage

**API**:
```java
WorkspaceWindow workspace = new WorkspaceWindow(stage, engine);
Scene scene = workspace.createScene();

// Access components
SceneInspector inspector = workspace.getSceneInspector();
TelemetryPane telemetry = workspace.getTelemetryPane();
TabPane centerTabs = workspace.getCenterTabPane();
ConsolePane console = workspace.getConsolePane();

// Update status (call at 10-30 Hz)
workspace.updateStatus();

// Save layout on exit
workspace.saveLayout();
```

### 2. ConsolePane.java
Professional console/log panel:
- **Log Levels**: INFO, WARNING, ERROR
- **Auto-scroll**: Toggle auto-scroll to latest message
- **Thread-safe**: Can log from any thread
- **Auto-trim**: Maintains max 1000 lines
- **Timestamps**: Each message has timestamp

**API**:
```java
ConsolePane console = new ConsolePane();

// Log messages
console.info("Engine initialized");
console.warning("Low memory");
console.error("Failed to load asset");
console.error("Exception occurred", throwable);

// Clear console
console.clear();
```

### 3. LayoutConfig.java
Layout persistence manager:
- **Window State**: Size, position, maximized state
- **Dividers**: Split pane positions
- **Visibility**: Pane show/hide states
- **Storage**: `~/.astraeus/workspace-layout.properties`

**API**:
```java
LayoutConfig config = LayoutConfig.load();

// Window properties
config.setWindowWidth(1600);
config.setWindowHeight(900);
double width = config.getWindowWidth(1600);

// Divider positions
config.setDividerPosition("main.horizontal", 0.75);
double pos = config.getDividerPosition("main.horizontal", 0.75);

// Pane visibility
config.setPaneVisible("console", true);
boolean visible = config.isPaneVisible("console", true);

// Save to disk
config.save();
```

## Integration

### AstraeusApp Integration

The main application has been updated to use the workspace:

```java
public class AstraeusApp extends Application {
    private WorkspaceWindow workspace;
    
    @Override
    public void start(Stage primaryStage) {
        // Create workspace
        workspace = new WorkspaceWindow(primaryStage, engine);
        
        // Create and show scene
        Scene scene = workspace.createScene();
        primaryStage.setScene(scene);
        primaryStage.show();
        
        // Start update loop for status bar
        startUpdateLoop();
    }
    
    @Override
    public void stop() {
        // Save layout on exit
        workspace.saveLayout();
        
        // Close engine
        if (engine != null) {
            engine.close();
        }
    }
}
```

## Features

### 1. Pane Visibility Toggle
- **View Menu**: Toggle each pane independently
- **Checkmarks**: Show current visibility state
- **Persistence**: State saved across sessions

### 2. Layout Persistence
Configuration saved to: `~/.astraeus/workspace-layout.properties`

**Saved Properties**:
- `window.width`, `window.height` - Window size
- `window.x`, `window.y` - Window position
- `window.maximized` - Maximized state
- `divider.main.horizontal` - Top/bottom split
- `divider.main.vertical` - Left/center split
- `divider.right.vertical` - Center/right split
- `pane.scene-inspector.visible` - Scene inspector visibility
- `pane.properties.visible` - Properties pane visibility
- `pane.console.visible` - Console visibility

### 3. Reset Layout
- **Menu**: View → Reset Layout
- **Confirmation**: Dialog before resetting
- **Defaults**: Restores to default positions

### 4. Status Bar
Real-time information:
- **Engine Status**: Running / Not Initialized
- **FPS**: Current frame rate (when telemetry enabled)
- **Memory**: Java heap usage (used / max)
- **Version**: Application version

## Usage Example

```java
// Create workspace
WorkspaceWindow workspace = new WorkspaceWindow(stage, engine);

// Get console for logging
ConsolePane console = workspace.getConsolePane();
console.info("Application started");

// Get scene inspector
SceneInspector inspector = workspace.getSceneInspector();
inspector.updateSelection(pickResult);

// Get telemetry pane
TelemetryPane telemetry = workspace.getTelemetryPane();
telemetry.update(); // Call at 10-30 Hz

// Add viewport tab
TabPane tabs = workspace.getCenterTabPane();
Tab viewportTab = new Tab("Viewport 1");
viewportTab.setContent(new FxViewport(engine));
tabs.getTabs().add(viewportTab);

// Update status bar (10-30 Hz)
workspace.updateStatus();

// Save on exit
workspace.saveLayout();
```

## Performance

### Update Rates
- **Status Bar**: 10 Hz (100ms interval)
- **Telemetry**: 10-30 Hz (throttled)
- **Console**: Batched updates (not per-message)

### Memory
- Console limited to 1000 lines (auto-trim)
- TelemetryPane reuses TableView rows
- No per-frame allocations

## Configuration File

Example `~/.astraeus/workspace-layout.properties`:

```properties
# Astraeus Workspace Layout Configuration
window.width=1600
window.height=900
window.x=100
window.y=50
window.maximized=false

# Dividers (0.0 to 1.0)
divider.main.horizontal=0.75
divider.main.vertical=0.20
divider.right.vertical=0.80

# Pane visibility
pane.scene-inspector.visible=true
pane.properties.visible=true
pane.console.visible=true
pane.telemetry.visible=false
```

## Requirements

- **Java**: 21+ (for FFM API)
- **JavaFX**: 21.0.1+
- **Maven**: 3.6+

## Building

```bash
# Full build
mvn clean compile

# Run application
mvn javafx:run

# Or use AstraeusApp.Starter
java -cp target/classes com.astraeus.ui.AstraeusApp$Starter
```

## Files

### Created/Updated Files:
- `java/src/main/java/com/astraeus/ui/WorkspaceWindow.java` (NEW)
- `java/src/main/java/com/astraeus/ui/ConsolePane.java` (NEW)
- `java/src/main/java/com/astraeus/ui/LayoutConfig.java` (NEW)
- `java/src/main/java/com/astraeus/ui/AstraeusApp.java` (UPDATED)

### Existing Components Used:
- `com.astraeus.tools.SceneInspector` - Left panel
- `com.astraeus.tools.TelemetryPane` - Right panel
- `com.astraeus.native_api.NativeEngine` - Engine integration

## Future Enhancements

1. **Multiple Viewports**: Add/remove viewport tabs dynamically
2. **Custom Panes**: Plugin system for additional tool panes
3. **Layouts**: Save/load named layout presets
4. **Themes**: Dark/light theme support
5. **Keyboard Shortcuts**: Hotkeys for common actions
6. **Detachable Panes**: Floating tool windows

## Troubleshooting

### Issue: Layout not saving
**Solution**: Check write permissions to `~/.astraeus/` directory

### Issue: Panes not resizing properly
**Solution**: Ensure divider positions are in range [0.0, 1.0]

### Issue: Console not updating
**Solution**: Verify Platform.runLater() is being called for UI updates

### Issue: Status bar not updating
**Solution**: Check that updateStatus() is being called in animation loop

## License

© 2024 Astraeus Project
