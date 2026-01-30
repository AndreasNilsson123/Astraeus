# Workspace Shell - Quick Reference

## API Cheat Sheet

### Creating the Workspace
```java
WorkspaceWindow workspace = new WorkspaceWindow(stage, null);
Scene scene = workspace.createScene();
stage.setScene(scene);
stage.show();
```

### Logging (Thread-Safe)
```java
ConsolePane console = workspace.getConsolePane();
console.info("Operation completed");
console.warning("Low memory");
console.error("Failed to load", exception);
console.clear();
```

### Engine Management
```java
// Initialize engine
NativeEngine engine = new NativeEngine(1280, 720, true);
workspace.setEngine(engine);

// Access engine-dependent components
if (engine != null && engine.isValid()) {
    // Safe to use engine
}
```

### Adding Viewport Tabs
```java
TabPane tabs = workspace.getCenterTabPane();
Tab viewportTab = new Tab("Viewport 1");
viewportTab.setContent(new FxViewport(engine));
tabs.getTabs().add(viewportTab);
```

### Updating Status Bar (10 Hz)
```java
AnimationTimer timer = new AnimationTimer() {
    @Override
    public void handle(long now) {
        workspace.updateStatus();
        workspace.updateTelemetry();
    }
};
timer.start();
```

### Accessing Components
```java
SceneInspector inspector = workspace.getSceneInspector();
TelemetryPane telemetry = workspace.getTelemetryPane();
ConsolePane console = workspace.getConsolePane();
TabPane tabs = workspace.getCenterTabPane();
```

### Layout Persistence
```java
// Automatically saved on window close
// Manually save:
workspace.saveLayout();

// Access configuration:
LayoutConfig config = LayoutConfig.load();
double width = config.getWindowWidth(1600);
config.setWindowWidth(1920);
config.save();
```

## Configuration File

Location: `~/.astraeus/workspace-layout.properties`

```properties
# Window
window.width=1600
window.height=900
window.x=100
window.y=50
window.maximized=false

# Dividers (0.0 to 1.0)
divider.main.horizontal=0.75
divider.main.vertical=0.20
divider.right.vertical=0.80

# Visibility
pane.scene-inspector.visible=true
pane.properties.visible=true
pane.console.visible=true
```

## Menu Actions

### File Menu
- Exit - Close application (saves layout)

### View Menu
- Scene Inspector - Toggle left pane
- Properties - Toggle right pane
- Console - Toggle bottom pane
- Reset Layout - Restore defaults

### Help Menu
- About - Application info

## Performance Guidelines

| Component | Update Rate | Notes |
|-----------|-------------|-------|
| Status Bar | 10 Hz | Throttle to avoid overhead |
| Telemetry | 10-30 Hz | Only when engine active |
| Console | Batched | Automatic, thread-safe |
| Layout Save | On close | Automatic |

## Common Patterns

### Initialization Sequence
```java
1. Create WorkspaceWindow(stage, null)
2. Show stage
3. User clicks "Initialize Engine"
4. Call workspace.setEngine(engine)
5. Start update loop
```

### Logging from Worker Thread
```java
new Thread(() -> {
    try {
        // Do work
        workspace.getConsolePane().info("Work started");
        // More work
        workspace.getConsolePane().info("Work completed");
    } catch (Exception e) {
        workspace.getConsolePane().error("Work failed", e);
    }
}).start();
```

### Handling Null Engine
```java
// Always check before using engine
if (workspace.getTelemetryPane() != null) {
    workspace.updateTelemetry();
}

// Or use the null-safe method
workspace.updateTelemetry(); // Handles null internally
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Layout not saving | Check write permissions to ~/.astraeus/ |
| Panes not resizing | Ensure divider positions are [0.0, 1.0] |
| Console not updating | Verify Platform.runLater() is being called |
| Status bar frozen | Check update loop is running |
| Memory leak | Don't recreate workspace, use setEngine() |

## File Locations

```
java/src/main/java/com/astraeus/ui/
├── WorkspaceWindow.java   (main workspace)
├── ConsolePane.java       (logging)
├── LayoutConfig.java      (persistence)
└── AstraeusApp.java       (application)

docs/
├── WORKSPACE_SHELL.md            (full documentation)
└── WORKSPACE_COMPLETION_REPORT.md (implementation report)

~/.astraeus/
└── workspace-layout.properties   (user config)
```

## Quick Checklist

Before committing code:
- [ ] Log important operations to console
- [ ] Check for null engine before use
- [ ] Throttle UI updates to 10-30 Hz
- [ ] Use Platform.runLater() for UI from threads
- [ ] Don't block UI thread
- [ ] Test layout persistence
- [ ] Verify memory doesn't leak
- [ ] Document any new panes

## Contact & Support

See full documentation in:
- `docs/WORKSPACE_SHELL.md` - Complete API reference
- `docs/WORKSPACE_COMPLETION_REPORT.md` - Implementation details

---
Quick Reference v0.1.0
