package com.astraeus.test;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.FrameStatsView;
import com.astraeus.native_api.PickingView;
import com.astraeus.rendering.FxViewport;
import com.astraeus.tools.SceneInspector;
import com.astraeus.tools.TelemetryOverlay;
import com.astraeus.tools.TelemetryPane;
import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.*;
import javafx.stage.Stage;

/**
 * Telemetry demonstration application.
 * 
 * Demonstrates the telemetry and profiling system:
 * - Real-time HUD overlay with FPS, CPU/GPU time, draw calls, triangles
 * - Detailed panel with per-pass performance breakdown
 * - Keyboard shortcut (F3) to toggle telemetry display
 * - Integration with existing picking and scene inspector
 * - Controlled update rate to avoid UI churn
 * 
 * KEYBOARD SHORTCUTS:
 * - F3: Toggle telemetry overlay
 * - T: Toggle telemetry panel
 * - E: Toggle telemetry collection (enable/disable)
 */
public class TelemetryDemoApp extends Application {
    
    private NativeEngine engine;
    private FxViewport viewport;
    private SceneInspector inspector;
    private TelemetryOverlay telemetryOverlay;
    private TelemetryPane telemetryPane;
    
    private Label statusLabel;
    
    // Reusable frame stats view (avoid per-frame allocations)
    private final FrameStatsView frameStats = new FrameStatsView();
    
    private long lastFrameTime;
    private long lastTelemetryUpdateTime;
    private static final long TELEMETRY_UPDATE_INTERVAL_NS = 33_000_000; // ~30 Hz
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - Telemetry Demo");
        
        try {
            // Initialize engine
            engine = new NativeEngine(1280, 720, true);
            
            // Enable telemetry by default
            engine.setTelemetryEnabled(true);
            
            // Create FxViewport with large max dimensions
            viewport = new FxViewport(engine, 2560, 1440, 1280, 720);
            viewport.setPrefSize(1280, 720);
            
            // Create telemetry overlay and position it over viewport
            telemetryOverlay = new TelemetryOverlay();
            telemetryOverlay.setVisible(true);
            
            // Create viewport container with overlay
            StackPane viewportContainer = new StackPane();
            viewportContainer.getChildren().add(viewport);
            viewportContainer.getChildren().add(telemetryOverlay);
            StackPane.setAlignment(telemetryOverlay, Pos.TOP_LEFT);
            StackPane.setMargin(telemetryOverlay, new Insets(10));
            
            // Create scene inspector
            inspector = new SceneInspector();
            
            // Create telemetry pane
            telemetryPane = new TelemetryPane(engine);
            telemetryPane.setVisible(false);  // Hidden by default
            
            // Setup picking callback
            viewport.setOnEntitySelected(this::handleEntitySelected);
            
            // Create UI
            BorderPane root = new BorderPane();
            
            // Top toolbar
            ToolBar toolbar = createToolbar();
            root.setTop(toolbar);
            
            // Center: viewport with overlay
            root.setCenter(viewportContainer);
            
            // Right: tabbed panel with inspector and telemetry
            TabPane rightPanel = new TabPane();
            rightPanel.setTabClosingPolicy(TabPane.TabClosingPolicy.UNAVAILABLE);
            
            Tab inspectorTab = new Tab("Inspector", inspector);
            Tab telemetryTab = new Tab("Telemetry", telemetryPane);
            
            rightPanel.getTabs().addAll(inspectorTab, telemetryTab);
            rightPanel.setMinWidth(320);
            rightPanel.setPrefWidth(400);
            
            root.setRight(rightPanel);
            
            // Bottom status bar
            HBox statusBar = createStatusBar();
            root.setBottom(statusBar);
            
            // Create scene
            Scene scene = new Scene(root, 1700, 900);
            
            // Setup keyboard shortcuts
            scene.addEventHandler(KeyEvent.KEY_PRESSED, this::handleKeyPress);
            
            primaryStage.setScene(scene);
            primaryStage.show();
            
            // Start render loop
            startRenderLoop();
            
            updateStatus("Ready - Press F3 to toggle telemetry overlay, T for telemetry panel");
            
        } catch (Exception e) {
            showError("Initialization Failed", e.getMessage());
            e.printStackTrace();
        }
    }
    
    private ToolBar createToolbar() {
        ToolBar toolbar = new ToolBar();
        
        Button createEntityButton = new Button("Create Entity");
        createEntityButton.setOnAction(e -> createTestEntity());
        
        Button clearSelectionButton = new Button("Clear Selection");
        clearSelectionButton.setOnAction(e -> clearSelection());
        
        Separator sep1 = new Separator();
        
        CheckBox overlayCheckBox = new CheckBox("Overlay (F3)");
        overlayCheckBox.setSelected(telemetryOverlay.isVisible());
        overlayCheckBox.setOnAction(e -> telemetryOverlay.setVisible(overlayCheckBox.isSelected()));
        
        CheckBox telemetryEnabledCheckBox = new CheckBox("Collect (E)");
        telemetryEnabledCheckBox.setSelected(engine.isTelemetryEnabled());
        telemetryEnabledCheckBox.setOnAction(e -> {
            engine.setTelemetryEnabled(telemetryEnabledCheckBox.isSelected());
            telemetryPane.setTelemetryEnabled(telemetryEnabledCheckBox.isSelected());
        });
        
        Separator sep2 = new Separator();
        
        Button resize800Button = new Button("800x600");
        resize800Button.setOnAction(e -> resizeViewport(800, 600));
        
        Button resize1080Button = new Button("1920x1080");
        resize1080Button.setOnAction(e -> resizeViewport(1920, 1080));
        
        Separator sep3 = new Separator();
        
        Button aboutButton = new Button("About");
        aboutButton.setOnAction(e -> showAbout());
        
        toolbar.getItems().addAll(
            createEntityButton,
            clearSelectionButton,
            sep1,
            new Label("Telemetry:"),
            overlayCheckBox,
            telemetryEnabledCheckBox,
            sep2,
            new Label("Resize:"),
            resize800Button,
            resize1080Button,
            sep3,
            aboutButton
        );
        
        return toolbar;
    }
    
    private HBox createStatusBar() {
        HBox statusBar = new HBox(10);
        statusBar.setPadding(new Insets(5));
        statusBar.setStyle("-fx-background-color: #f0f0f0;");
        
        statusLabel = new Label("Ready");
        
        Label versionLabel = new Label("Astraeus v0.1.0 - Telemetry Demo");
        versionLabel.setStyle("-fx-text-fill: #666666;");
        
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        
        statusBar.getChildren().addAll(statusLabel, spacer, versionLabel);
        
        return statusBar;
    }
    
    private void startRenderLoop() {
        lastFrameTime = System.nanoTime();
        lastTelemetryUpdateTime = lastFrameTime;
        
        AnimationTimer timer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                try {
                    // Calculate delta time
                    double deltaTime = (now - lastFrameTime) / 1_000_000_000.0;
                    lastFrameTime = now;
                    
                    // Render frame
                    engine.beginFrame(deltaTime);
                    engine.endFrame();
                    
                    // Update display
                    viewport.updateDisplay();
                    
                    // Update telemetry at controlled rate (30 Hz)
                    if (now - lastTelemetryUpdateTime >= TELEMETRY_UPDATE_INTERVAL_NS) {
                        updateTelemetry();
                        lastTelemetryUpdateTime = now;
                    }
                    
                } catch (Exception e) {
                    System.err.println("Render loop error: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        };
        
        timer.start();
    }
    
    /**
     * Update telemetry UI components.
     * Called at controlled rate (30 Hz) to avoid UI churn.
     */
    private void updateTelemetry() {
        if (engine.isTelemetryEnabled()) {
            // Get frame stats (reuses frameStats instance)
            engine.getFrameStats(frameStats);
            
            // Update overlay if visible
            if (telemetryOverlay.isVisible()) {
                telemetryOverlay.update(frameStats);
            }
            
            // Update detailed pane if visible
            if (telemetryPane.isVisible()) {
                telemetryPane.update();
            }
        }
    }
    
    private void handleKeyPress(KeyEvent event) {
        if (event.getCode() == KeyCode.F3) {
            // Toggle telemetry overlay
            telemetryOverlay.setVisible(!telemetryOverlay.isVisible());
            updateStatus("Telemetry overlay " + (telemetryOverlay.isVisible() ? "enabled" : "disabled"));
            event.consume();
        } else if (event.getCode() == KeyCode.T) {
            // Toggle telemetry pane visibility
            telemetryPane.setVisible(!telemetryPane.isVisible());
            updateStatus("Telemetry panel " + (telemetryPane.isVisible() ? "shown" : "hidden"));
            event.consume();
        } else if (event.getCode() == KeyCode.E) {
            // Toggle telemetry collection
            boolean newState = !engine.isTelemetryEnabled();
            engine.setTelemetryEnabled(newState);
            telemetryPane.setTelemetryEnabled(newState);
            updateStatus("Telemetry collection " + (newState ? "enabled" : "disabled"));
            event.consume();
        }
    }
    
    private void handleEntitySelected(PickingView pickResult) {
        // Update inspector with pick result
        inspector.updateSelection(pickResult);
        
        // Update status
        if (pickResult.hasValidEntity()) {
            updateStatus(String.format("Selected Entity #%d at (%.2f, %.2f, %.2f)",
                                      pickResult.getEntityId(),
                                      pickResult.getWorldX(),
                                      pickResult.getWorldY(),
                                      pickResult.getWorldZ()));
        } else {
            updateStatus("Click on an entity to select it");
        }
    }
    
    private void createTestEntity() {
        try {
            int entityId = engine.createEntity();
            updateStatus("Created entity #" + entityId);
        } catch (Exception e) {
            showError("Failed to Create Entity", e.getMessage());
        }
    }
    
    private void clearSelection() {
        viewport.clearSelection();
        inspector.clearSelection();
        updateStatus("Selection cleared");
    }
    
    private void resizeViewport(int width, int height) {
        try {
            viewport.resizeViewport(width, height);
            updateStatus("Viewport resized to " + width + "x" + height);
        } catch (Exception e) {
            showError("Resize Failed", e.getMessage());
        }
    }
    
    private void updateStatus(String message) {
        statusLabel.setText(message);
    }
    
    private void showAbout() {
        Alert alert = new Alert(Alert.AlertType.INFORMATION);
        alert.setTitle("About Astraeus Telemetry Demo");
        alert.setHeaderText("Astraeus 3D Visualization Engine");
        alert.setContentText(
            "Telemetry Demonstration Application\n\n" +
            "Features:\n" +
            "- Real-time HUD overlay with FPS, CPU/GPU time, draw calls, triangles\n" +
            "- Detailed panel with per-pass performance breakdown\n" +
            "- Keyboard shortcuts for quick access\n" +
            "- Integration with existing picking and scene inspector\n\n" +
            "Keyboard Shortcuts:\n" +
            "  F3 - Toggle telemetry overlay\n" +
            "  T  - Toggle telemetry panel\n" +
            "  E  - Toggle telemetry collection\n\n" +
            "Version: 0.1.0\n" +
            "Task: C1 - Telemetry & Profiling"
        );
        alert.showAndWait();
    }
    
    private void showError(String title, String message) {
        Alert alert = new Alert(Alert.AlertType.ERROR);
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(message);
        alert.showAndWait();
    }
    
    @Override
    public void stop() {
        if (engine != null) {
            engine.close();
        }
    }
    
    public static void main(String[] args) {
        launch(args);
    }

    static class Starter {
        public static void main(String[] args) {
            TelemetryDemoApp.main(args);
        }
    }
}
