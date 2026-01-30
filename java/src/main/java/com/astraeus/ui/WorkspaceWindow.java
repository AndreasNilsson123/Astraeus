package com.astraeus.ui;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.tools.SceneInspector;
import com.astraeus.tools.TelemetryPane;
import javafx.geometry.Orientation;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

/**
 * Professional workspace window with docking-like layout.
 * 
 * Layout structure:
 * <pre>
 * ┌─────────────────────────────────────────────┐
 * │ Menu Bar                                     │
 * ├──────┬────────────────────────────┬─────────┤
 * │      │                            │         │
 * │ Left │      Center Viewport       │ Right   │
 * │Scene │         (TabPane)          │Properties│
 * │Insp. │                            │         │
 * │      │                            │         │
 * ├──────┴────────────────────────────┴─────────┤
 * │ Bottom Console / Log Pane                   │
 * ├─────────────────────────────────────────────┤
 * │ Status Bar                                  │
 * └─────────────────────────────────────────────┘
 * </pre>
 * 
 * Features:
 * - Resizable split panes
 * - Toggleable panes via View menu
 * - Layout persistence (window size, dividers, visibility)
 * - Status bar with engine info
 * - Tab-based center viewport area
 * 
 * USAGE:
 * <pre>
 * WorkspaceWindow workspace = new WorkspaceWindow(stage, engine);
 * Scene scene = workspace.createScene();
 * stage.setScene(scene);
 * </pre>
 */
public class WorkspaceWindow {
    
    private final Stage stage;
    private NativeEngine engine;
    private final LayoutConfig layoutConfig;
    
    // Layout components
    private BorderPane root;
    private SplitPane mainHorizontalSplit;
    private SplitPane mainVerticalSplit;
    private SplitPane rightVerticalSplit;
    
    // Panes
    private SceneInspector sceneInspector;
    private TelemetryPane telemetryPane;
    private TabPane centerTabPane;
    private ConsolePane consolePane;
    
    // Menu items for pane visibility
    private CheckMenuItem sceneInspectorMenuItem;
    private CheckMenuItem propertiesMenuItem;
    private CheckMenuItem consoleMenuItem;
    
    // Status bar
    private Label engineStatusLabel;
    private Label fpsLabel;
    private Label memoryLabel;
    
    public WorkspaceWindow(Stage stage, NativeEngine engine) {
        this.stage = stage;
        this.engine = engine;
        this.layoutConfig = LayoutConfig.load();
        
        buildUI();
        setupMenuBar();
        restoreLayout();
        setupShutdownHook();
    }
    
    /**
     * Set or update the engine reference.
     * This allows the engine to be initialized after the workspace is created.
     * 
     * @param engine Native engine instance (can be null)
     */
    public void setEngine(NativeEngine engine) {
        this.engine = engine;
        updateStatus();
    }
    
    /**
     * Build the UI layout.
     */
    private void buildUI() {
        root = new BorderPane();
        
        // === CENTER: Main workspace area ===
        buildMainWorkspace();
        
        // === BOTTOM: Status bar ===
        HBox statusBar = buildStatusBar();
        root.setBottom(statusBar);
    }
    
    /**
     * Build main workspace with split panes.
     */
    private void buildMainWorkspace() {
        // Left: Scene Inspector
        sceneInspector = new SceneInspector();
        
        // Center: Tabbed viewport area
        centerTabPane = new TabPane();
        centerTabPane.setTabClosingPolicy(TabPane.TabClosingPolicy.ALL_TABS);
        
        // Add a welcome tab
        Tab welcomeTab = new Tab("Welcome");
        welcomeTab.setClosable(false);
        VBox welcomeContent = new VBox(20);
        welcomeContent.setStyle("-fx-padding: 40; -fx-alignment: center;");
        Label titleLabel = new Label("Astraeus Visualization Engine");
        titleLabel.setStyle("-fx-font-size: 24; -fx-font-weight: bold;");
        Label subtitleLabel = new Label("Professional 3D visualization for simulation data");
        subtitleLabel.setStyle("-fx-font-size: 14; -fx-text-fill: #666666;");
        welcomeContent.getChildren().addAll(titleLabel, subtitleLabel);
        welcomeTab.setContent(welcomeContent);
        centerTabPane.getTabs().add(welcomeTab);
        
        // Right: Properties / Telemetry pane
        // Note: TelemetryPane requires engine, so we create a placeholder region if engine is null
        Region propertiesPane;
        if (engine != null) {
            telemetryPane = new TelemetryPane(engine);
            propertiesPane = telemetryPane;
        } else {
            // Create placeholder until engine is available
            VBox placeholder = new VBox(10);
            placeholder.setStyle("-fx-padding: 10; -fx-background-color: #f5f5f5;");
            Label label = new Label("Properties");
            label.setStyle("-fx-font-size: 16; -fx-font-weight: bold;");
            Label info = new Label("Initialize engine to view properties");
            info.setStyle("-fx-text-fill: #666666;");
            placeholder.getChildren().addAll(label, info);
            placeholder.setMinWidth(300);
            placeholder.setPrefWidth(350);
            propertiesPane = placeholder;
            telemetryPane = null;
        }
        
        // Bottom: Console pane
        consolePane = new ConsolePane();
        
        // === Split Pane Structure ===
        
        // Right vertical split (center + right properties)
        rightVerticalSplit = new SplitPane();
        rightVerticalSplit.setOrientation(Orientation.HORIZONTAL);
        rightVerticalSplit.getItems().addAll(centerTabPane, propertiesPane);
        
        // Main vertical split (left + right)
        mainVerticalSplit = new SplitPane();
        mainVerticalSplit.setOrientation(Orientation.HORIZONTAL);
        mainVerticalSplit.getItems().addAll(sceneInspector, rightVerticalSplit);
        
        // Main horizontal split (top + bottom console)
        mainHorizontalSplit = new SplitPane();
        mainHorizontalSplit.setOrientation(Orientation.VERTICAL);
        mainHorizontalSplit.getItems().addAll(mainVerticalSplit, consolePane);
        
        root.setCenter(mainHorizontalSplit);
    }
    
    /**
     * Setup menu bar with View menu for pane toggles.
     */
    private void setupMenuBar() {
        MenuBar menuBar = new MenuBar();
        
        // === File Menu ===
        Menu fileMenu = new Menu("File");
        
        MenuItem exitItem = new MenuItem("Exit");
        exitItem.setOnAction(e -> {
            saveLayout();
            stage.close();
        });
        
        fileMenu.getItems().addAll(exitItem);
        
        // === View Menu ===
        Menu viewMenu = new Menu("View");
        
        sceneInspectorMenuItem = new CheckMenuItem("Scene Inspector");
        sceneInspectorMenuItem.setSelected(layoutConfig.isPaneVisible("scene-inspector", true));
        sceneInspectorMenuItem.setOnAction(e -> togglePane("scene-inspector", sceneInspector, sceneInspectorMenuItem));
        
        propertiesMenuItem = new CheckMenuItem("Properties");
        propertiesMenuItem.setSelected(layoutConfig.isPaneVisible("properties", true));
        propertiesMenuItem.setOnAction(e -> togglePane("properties", telemetryPane, propertiesMenuItem));
        
        consoleMenuItem = new CheckMenuItem("Console");
        consoleMenuItem.setSelected(layoutConfig.isPaneVisible("console", true));
        consoleMenuItem.setOnAction(e -> togglePane("console", consolePane, consoleMenuItem));
        
        viewMenu.getItems().addAll(
            sceneInspectorMenuItem,
            propertiesMenuItem,
            consoleMenuItem,
            new SeparatorMenuItem(),
            createResetLayoutMenuItem()
        );
        
        // === Help Menu ===
        Menu helpMenu = new Menu("Help");
        
        MenuItem aboutItem = new MenuItem("About");
        aboutItem.setOnAction(e -> showAbout());
        
        helpMenu.getItems().addAll(aboutItem);
        
        // === Add menus to bar ===
        menuBar.getMenus().addAll(fileMenu, viewMenu, helpMenu);
        root.setTop(menuBar);
    }
    
    /**
     * Create "Reset Layout" menu item.
     */
    private MenuItem createResetLayoutMenuItem() {
        MenuItem resetItem = new MenuItem("Reset Layout");
        resetItem.setOnAction(e -> {
            Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
            alert.setTitle("Reset Layout");
            alert.setHeaderText("Reset workspace layout to defaults?");
            alert.setContentText("This will restore default window size and pane positions.");
            
            alert.showAndWait().ifPresent(response -> {
                if (response == ButtonType.OK) {
                    resetLayout();
                }
            });
        });
        return resetItem;
    }
    
    /**
     * Build status bar.
     */
    private HBox buildStatusBar() {
        HBox statusBar = new HBox(15);
        statusBar.setStyle("-fx-padding: 5 10; -fx-background-color: #f0f0f0; -fx-border-color: #cccccc; -fx-border-width: 1 0 0 0;");
        
        engineStatusLabel = new Label("Engine: Not Initialized");
        engineStatusLabel.setStyle("-fx-font-size: 11;");
        
        fpsLabel = new Label("FPS: --");
        fpsLabel.setStyle("-fx-font-size: 11;");
        
        memoryLabel = new Label("Memory: --");
        memoryLabel.setStyle("-fx-font-size: 11;");
        
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        
        Label versionLabel = new Label("Astraeus v0.1.0");
        versionLabel.setStyle("-fx-font-size: 11; -fx-text-fill: #666666;");
        
        statusBar.getChildren().addAll(
            engineStatusLabel,
            new Separator(Orientation.VERTICAL),
            fpsLabel,
            new Separator(Orientation.VERTICAL),
            memoryLabel,
            spacer,
            versionLabel
        );
        
        return statusBar;
    }
    
    /**
     * Toggle pane visibility.
     */
    private void togglePane(String paneName, Region pane, CheckMenuItem menuItem) {
        boolean visible = menuItem.isSelected();
        layoutConfig.setPaneVisible(paneName, visible);
        
        SplitPane parentSplit = getParentSplit(pane);
        if (parentSplit != null) {
            if (visible) {
                // Add pane back if not present
                if (!parentSplit.getItems().contains(pane)) {
                    addPaneToCorrectPosition(parentSplit, pane, paneName);
                }
            } else {
                // Remove pane
                parentSplit.getItems().remove(pane);
            }
        }
    }
    
    /**
     * Get parent split pane for a given pane.
     */
    private SplitPane getParentSplit(Region pane) {
        if (pane == sceneInspector) {
            return mainVerticalSplit;
        } else if (pane == telemetryPane) {
            return rightVerticalSplit;
        } else if (pane == consolePane) {
            return mainHorizontalSplit;
        }
        return null;
    }
    
    /**
     * Add pane to correct position in split.
     */
    private void addPaneToCorrectPosition(SplitPane split, Region pane, String paneName) {
        if (pane == sceneInspector) {
            // Add at start
            if (!split.getItems().contains(pane)) {
                split.getItems().add(0, pane);
            }
        } else if (pane == telemetryPane) {
            // Add at end
            if (!split.getItems().contains(pane)) {
                split.getItems().add(pane);
            }
        } else if (pane == consolePane) {
            // Add at end
            if (!split.getItems().contains(pane)) {
                split.getItems().add(pane);
            }
        }
    }
    
    /**
     * Restore layout from configuration.
     */
    private void restoreLayout() {
        // Window size
        double width = layoutConfig.getWindowWidth(1600);
        double height = layoutConfig.getWindowHeight(900);
        stage.setWidth(width);
        stage.setHeight(height);
        
        // Window position (if saved)
        double x = layoutConfig.getWindowX(-1);
        double y = layoutConfig.getWindowY(-1);
        if (x >= 0 && y >= 0) {
            stage.setX(x);
            stage.setY(y);
        }
        
        // Maximized state
        boolean maximized = layoutConfig.isWindowMaximized(false);
        stage.setMaximized(maximized);
        
        // Divider positions (set after scene is shown)
        stage.setOnShown(e -> restoreDividerPositions());
        
        // Pane visibility - remove panes that should be hidden
        // Note: Menu items already reflect correct state from initialization
        if (!sceneInspectorMenuItem.isSelected()) {
            mainVerticalSplit.getItems().remove(sceneInspector);
        }
        if (!propertiesMenuItem.isSelected()) {
            rightVerticalSplit.getItems().remove(telemetryPane);
        }
        if (!consoleMenuItem.isSelected()) {
            mainHorizontalSplit.getItems().remove(consolePane);
        }
        
        consolePane.info("Workspace layout restored");
    }
    
    /**
     * Restore divider positions.
     * Must be called after scene is shown for proper layout.
     */
    private void restoreDividerPositions() {
        double leftPos = layoutConfig.getDividerPosition("main.vertical", 0.20);
        double rightPos = layoutConfig.getDividerPosition("right.vertical", 0.80);
        double bottomPos = layoutConfig.getDividerPosition("main.horizontal", 0.75);
        
        mainVerticalSplit.setDividerPositions(leftPos);
        rightVerticalSplit.setDividerPositions(rightPos);
        mainHorizontalSplit.setDividerPositions(bottomPos);
    }
    
    /**
     * Save current layout to configuration.
     */
    public void saveLayout() {
        // Window properties
        if (!stage.isMaximized()) {
            layoutConfig.setWindowWidth(stage.getWidth());
            layoutConfig.setWindowHeight(stage.getHeight());
            layoutConfig.setWindowX(stage.getX());
            layoutConfig.setWindowY(stage.getY());
        }
        layoutConfig.setWindowMaximized(stage.isMaximized());
        
        // Divider positions
        if (mainVerticalSplit.getDividers().size() > 0) {
            layoutConfig.setDividerPosition("main.vertical", 
                mainVerticalSplit.getDividerPositions()[0]);
        }
        if (rightVerticalSplit.getDividers().size() > 0) {
            layoutConfig.setDividerPosition("right.vertical", 
                rightVerticalSplit.getDividerPositions()[0]);
        }
        if (mainHorizontalSplit.getDividers().size() > 0) {
            layoutConfig.setDividerPosition("main.horizontal", 
                mainHorizontalSplit.getDividerPositions()[0]);
        }
        
        // Save to disk
        layoutConfig.save();
        consolePane.info("Workspace layout saved");
    }
    
    /**
     * Reset layout to defaults.
     */
    private void resetLayout() {
        // Reset to default positions
        mainVerticalSplit.setDividerPositions(0.20);
        rightVerticalSplit.setDividerPositions(0.80);
        mainHorizontalSplit.setDividerPositions(0.75);
        
        // Show all panes
        sceneInspectorMenuItem.setSelected(true);
        propertiesMenuItem.setSelected(true);
        consoleMenuItem.setSelected(true);
        
        // Ensure panes are visible
        if (!mainVerticalSplit.getItems().contains(sceneInspector)) {
            mainVerticalSplit.getItems().add(0, sceneInspector);
        }
        if (!rightVerticalSplit.getItems().contains(telemetryPane)) {
            rightVerticalSplit.getItems().add(telemetryPane);
        }
        if (!mainHorizontalSplit.getItems().contains(consolePane)) {
            mainHorizontalSplit.getItems().add(consolePane);
        }
        
        // Reset window size
        stage.setWidth(1600);
        stage.setHeight(900);
        stage.centerOnScreen();
        
        consolePane.info("Layout reset to defaults");
    }
    
    /**
     * Setup shutdown hook to save layout on exit.
     */
    private void setupShutdownHook() {
        stage.setOnCloseRequest(e -> {
            saveLayout();
            consolePane.info("Shutting down...");
        });
    }
    
    /**
     * Show about dialog.
     */
    private void showAbout() {
        Alert alert = new Alert(Alert.AlertType.INFORMATION);
        alert.setTitle("About Astraeus");
        alert.setHeaderText("Astraeus 3D Visualization Engine");
        alert.setContentText(
            "Version: 0.1.0\n\n" +
            "A professional, scalable 3D visualization engine\n" +
            "for high-performance visualization of externally simulated data.\n\n" +
            "Architecture:\n" +
            "- C++ core engine (rendering, scene, data ingestion)\n" +
            "- Java frontend (JavaFX UI, tooling)\n" +
            "- FFM integration layer\n\n" +
            "© 2024 Astraeus Project"
        );
        alert.showAndWait();
    }
    
    // ==================== Public API ====================
    
    /**
     * Get the root pane.
     */
    public BorderPane getRoot() {
        return root;
    }
    
    /**
     * Get scene inspector.
     */
    public SceneInspector getSceneInspector() {
        return sceneInspector;
    }
    
    /**
     * Get telemetry pane.
     */
    public TelemetryPane getTelemetryPane() {
        return telemetryPane;
    }
    
    /**
     * Get center tab pane (for adding viewport tabs).
     */
    public TabPane getCenterTabPane() {
        return centerTabPane;
    }
    
    /**
     * Get console pane.
     */
    public ConsolePane getConsolePane() {
        return consolePane;
    }
    
    /**
     * Update status bar (call periodically, e.g., 10-30 Hz).
     */
    public void updateStatus() {
        if (engine != null && engine.isValid()) {
            engineStatusLabel.setText("Engine: Running");
            
            // Update FPS if telemetry is enabled
            if (engine.isTelemetryEnabled()) {
                try {
                    var stats = engine.getTelemetryStats();
                    fpsLabel.setText(String.format("FPS: %.1f", stats.getFPS()));
                } catch (Exception e) {
                    fpsLabel.setText("FPS: --");
                }
            }
        } else {
            engineStatusLabel.setText("Engine: Not Initialized");
            fpsLabel.setText("FPS: --");
        }
        
        // Memory usage
        Runtime runtime = Runtime.getRuntime();
        long usedMemory = (runtime.totalMemory() - runtime.freeMemory()) / (1024 * 1024);
        long maxMemory = runtime.maxMemory() / (1024 * 1024);
        memoryLabel.setText(String.format("Memory: %d / %d MB", usedMemory, maxMemory));
    }
    
    /**
     * Update telemetry pane if available.
     * Call at 10-30 Hz when engine is active.
     */
    public void updateTelemetry() {
        if (telemetryPane != null && engine != null && engine.isValid()) {
            telemetryPane.update();
        }
    }
    
    /**
     * Create JavaFX Scene with the workspace.
     */
    public javafx.scene.Scene createScene() {
        return new javafx.scene.Scene(root);
    }
}
