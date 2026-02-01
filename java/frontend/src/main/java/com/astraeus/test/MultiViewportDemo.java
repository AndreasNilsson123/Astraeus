package com.astraeus.test;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.model.PickResult;
import com.astraeus.rendering.FxViewport;
import com.astraeus.rendering.ViewportController;
import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

/**
 * Multi-Viewport demonstration application.
 * 
 * Demonstrates:
 * - Two independent viewports with separate camera controls
 * - Input routing isolation (each viewport responds independently)
 * - Overlay stack functionality
 * - Camera controller modes (orbit/fly/pan)
 * - No per-frame allocations
 * 
 * CONTROLS:
 * - Left mouse: Rotate/look camera
 * - Middle mouse: Pan
 * - Scroll: Zoom
 * - WASD: Move (fly mode)
 * - QE: Up/down (fly mode)
 * - 1/2/3: Switch camera modes
 * - F1: Toggle camera info
 * - F2: Toggle telemetry
 * - Click: Pick entity
 * 
 * ACCEPTANCE CRITERIA:
 * - Two viewports can exist and behave independently
 * - Each viewport has its own camera controller
 * - Input events don't leak between viewports
 * - No per-frame allocations in hot path
 */
public class MultiViewportDemo extends Application {
    
    private NativeEngine engine1;
    private NativeEngine engine2;
    private FxViewport viewport1;
    private FxViewport viewport2;
    
    private Label statusLabel;
    private Label fps1Label;
    private Label fps2Label;
    private Label activeViewportLabel;
    
    private long startTime;
    private long lastFrameTime;
    private int frameCount = 0;
    private double fps = 0.0;
    
    private FxViewport activeViewport;
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - Multi-Viewport Demo");
        
        try {
            // Initialize two separate engines
            engine1 = new NativeEngine(1280, 720, true);
            engine2 = new NativeEngine(1280, 720, true);
            
            // Create two viewports
            viewport1 = new FxViewport(engine1, 2560, 1440, 800, 600);
            viewport1.setPrefSize(800, 600);
            viewport1.getController().setMode(ViewportController.Mode.ORBIT);
            viewport1.setOnEntitySelected(result -> handlePickViewport1(result));
            
            viewport2 = new FxViewport(engine2, 2560, 1440, 800, 600);
            viewport2.setPrefSize(800, 600);
            viewport2.getController().setMode(ViewportController.Mode.FLY);
            viewport2.setOnEntitySelected(result -> handlePickViewport2(result));
            
            activeViewport = viewport1;
            
            // Create UI
            BorderPane root = new BorderPane();
            
            // Top toolbar
            ToolBar toolbar = createToolbar();
            root.setTop(toolbar);
            
            // Center: Split pane with two viewports
            SplitPane splitPane = new SplitPane();
            splitPane.setOrientation(javafx.geometry.Orientation.HORIZONTAL);
            
            VBox viewport1Container = createViewportContainer("Viewport 1 (Orbit Mode)", viewport1);
            VBox viewport2Container = createViewportContainer("Viewport 2 (Fly Mode)", viewport2);
            
            splitPane.getItems().addAll(viewport1Container, viewport2Container);
            splitPane.setDividerPositions(0.5);
            
            root.setCenter(splitPane);
            
            // Bottom: Status bar
            HBox statusBar = createStatusBar();
            root.setBottom(statusBar);
            
            // Create scene
            Scene scene = new Scene(root, 1700, 900);
            primaryStage.setScene(scene);
            primaryStage.show();
            
            // Track which viewport has focus
            viewport1.focusedProperty().addListener((obs, wasFocused, isNowFocused) -> {
                if (isNowFocused) {
                    activeViewport = viewport1;
                    updateActiveViewportLabel();
                    viewport1.setStyle("-fx-border-color: yellow; -fx-border-width: 2;");
                } else {
                    viewport1.setStyle("");
                }
            });
            
            viewport2.focusedProperty().addListener((obs, wasFocused, isNowFocused) -> {
                if (isNowFocused) {
                    activeViewport = viewport2;
                    updateActiveViewportLabel();
                    viewport2.setStyle("-fx-border-color: yellow; -fx-border-width: 2;");
                } else {
                    viewport2.setStyle("");
                }
            });
            
            // Start render loop
            startRenderLoop();
            
            updateStatus("Ready - Click viewports to interact, use mouse/keyboard to control camera");
            
        } catch (Exception e) {
            showError("Initialization Failed", e.getMessage());
            e.printStackTrace();
        }
    }
    
    private VBox createViewportContainer(String title, FxViewport viewport) {
        VBox container = new VBox(5);
        container.setPadding(new Insets(5));
        
        Label titleLabel = new Label(title);
        titleLabel.setStyle("-fx-font-weight: bold; -fx-font-size: 14px;");
        
        container.getChildren().addAll(titleLabel, viewport);
        VBox.setVgrow(viewport, Priority.ALWAYS);
        
        return container;
    }
    
    private ToolBar createToolbar() {
        ToolBar toolbar = new ToolBar();
        
        // Camera mode buttons
        Label modeLabel = new Label("Camera Mode:");
        
        Button orbitButton = new Button("Orbit (1)");
        orbitButton.setOnAction(e -> setActiveViewportMode(ViewportController.Mode.ORBIT));
        
        Button flyButton = new Button("Fly (2)");
        flyButton.setOnAction(e -> setActiveViewportMode(ViewportController.Mode.FLY));
        
        Button panButton = new Button("Pan (3)");
        panButton.setOnAction(e -> setActiveViewportMode(ViewportController.Mode.PAN));
        
        Separator sep1 = new Separator();
        
        // Overlay toggles
        Button cameraInfoButton = new Button("Camera Info (F1)");
        cameraInfoButton.setOnAction(e -> {
            if (activeViewport != null) {
                activeViewport.getOverlayStack().toggleOverlay("camera-info");
            }
        });
        
        Button telemetryButton = new Button("Telemetry (F2)");
        telemetryButton.setOnAction(e -> {
            if (activeViewport != null) {
                activeViewport.getOverlayStack().toggleOverlay("telemetry");
            }
        });
        
        Separator sep2 = new Separator();
        
        // Entity controls
        Button createEntityButton = new Button("Create Entity");
        createEntityButton.setOnAction(e -> createEntity());
        
        Button clearButton = new Button("Clear Selection");
        clearButton.setOnAction(e -> clearSelections());
        
        Separator sep3 = new Separator();
        
        Button aboutButton = new Button("About");
        aboutButton.setOnAction(e -> showAbout());
        
        toolbar.getItems().addAll(
            modeLabel, orbitButton, flyButton, panButton,
            sep1,
            cameraInfoButton, telemetryButton,
            sep2,
            createEntityButton, clearButton,
            sep3,
            aboutButton
        );
        
        return toolbar;
    }
    
    private HBox createStatusBar() {
        HBox statusBar = new HBox(20);
        statusBar.setPadding(new Insets(5));
        statusBar.setStyle("-fx-background-color: #f0f0f0;");
        
        statusLabel = new Label("Ready");
        fps1Label = new Label("VP1 FPS: 0.0");
        fps2Label = new Label("VP2 FPS: 0.0");
        activeViewportLabel = new Label("Active: Viewport 1");
        
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        
        statusBar.getChildren().addAll(
            new Label("Status:"),
            statusLabel,
            spacer,
            activeViewportLabel,
            new Separator(),
            fps1Label,
            new Separator(),
            fps2Label
        );
        
        return statusBar;
    }
    
    private void startRenderLoop() {
        startTime = System.nanoTime();
        lastFrameTime = startTime;
        
        AnimationTimer timer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                try {
                    double deltaTime = (now - lastFrameTime) / 1_000_000_000.0;
                    lastFrameTime = now;
                    
                    // Update viewports
                    viewport1.update(deltaTime);
                    viewport2.update(deltaTime);
                    
                    // Render both engines
                    engine1.beginFrame(deltaTime);
                    engine1.endFrame();
                    
                    engine2.beginFrame(deltaTime);
                    engine2.endFrame();
                    
                    // Update displays
                    viewport1.updateDisplay();
                    viewport2.updateDisplay();
                    
                    // Update FPS
                    frameCount++;
                    double elapsed = (now - startTime) / 1_000_000_000.0;
                    if (elapsed >= 1.0) {
                        fps = frameCount / elapsed;
                        fps1Label.setText(String.format("VP1 FPS: %.1f", fps));
                        fps2Label.setText(String.format("VP2 FPS: %.1f", fps));
                        frameCount = 0;
                        startTime = now;
                    }
                    
                } catch (Exception e) {
                    System.err.println("Render loop error: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        };
        
        timer.start();
    }
    
    private void setActiveViewportMode(ViewportController.Mode mode) {
        if (activeViewport != null) {
            activeViewport.getController().setMode(mode);
            updateStatus("Camera mode changed to " + mode);
        }
    }
    
    private void handlePickViewport1(PickResult result) {
        if (result.hasValidEntity()) {
            updateStatus("VP1: Selected entity #" + result.getEntityId());
        }
    }
    
    private void handlePickViewport2(PickResult result) {
        if (result.hasValidEntity()) {
            updateStatus("VP2: Selected entity #" + result.getEntityId());
        }
    }
    
    private void createEntity() {
        try {
            if (activeViewport != null) {
                NativeEngine engine = activeViewport.getEngine();
                int entityId = engine.createEntity();
                updateStatus("Created entity #" + entityId + " in active viewport");
            }
        } catch (Exception e) {
            showError("Failed to Create Entity", e.getMessage());
        }
    }
    
    private void clearSelections() {
        viewport1.clearSelection();
        viewport2.clearSelection();
        updateStatus("Selections cleared");
    }
    
    private void updateStatus(String message) {
        statusLabel.setText(message);
    }
    
    private void updateActiveViewportLabel() {
        if (activeViewport == viewport1) {
            activeViewportLabel.setText("Active: Viewport 1");
        } else if (activeViewport == viewport2) {
            activeViewportLabel.setText("Active: Viewport 2");
        }
    }
    
    private void showAbout() {
        Alert alert = new Alert(Alert.AlertType.INFORMATION);
        alert.setTitle("About Multi-Viewport Demo");
        alert.setHeaderText("Astraeus Multi-Viewport Demo");
        alert.setContentText(
            "Demonstrates multiple independent viewports with:\n\n" +
            "✓ Separate camera controllers\n" +
            "✓ Input isolation per viewport\n" +
            "✓ Overlay stack (HUD, selection, gizmos)\n" +
            "✓ Multiple camera modes (orbit/fly/pan)\n" +
            "✓ Zero per-frame allocations\n\n" +
            "Controls:\n" +
            "- Left mouse: Rotate/look\n" +
            "- Middle mouse: Pan\n" +
            "- Scroll: Zoom\n" +
            "- WASD: Move (fly mode)\n" +
            "- QE: Up/down (fly mode)\n" +
            "- 1/2/3: Switch modes\n" +
            "- F1: Camera info\n" +
            "- F2: Telemetry\n\n" +
            "Task: J2 - Viewport Framework v2"
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
        if (engine1 != null) {
            engine1.close();
        }
        if (engine2 != null) {
            engine2.close();
        }
    }
    
    public static void main(String[] args) {
        launch(args);
    }
    
    static class Starter {
        public static void main(String[] args) {
            MultiViewportDemo.main(args);
        }
    }
}
