package com.astraeus.ui;

import com.astraeus.native_api.NativeEngine;
import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

/**
 * Main JavaFX application for Astraeus visualization engine.
 * 
 * Features:
 * - Professional workspace with docking-like layout
 * - Scene inspector, telemetry, and console panes
 * - Layout persistence across sessions
 * - Toolbar for common actions
 */
public class AstraeusApp extends Application {
    
    private NativeEngine engine;
    private WorkspaceWindow workspace;
    private AnimationTimer updateTimer;
    
    private long lastStatusUpdate = 0;
    private static final long STATUS_UPDATE_INTERVAL_NS = 100_000_000; // 100ms (10 Hz)
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - 3D Visualization Engine");
        
        // Initialize workspace without engine first
        workspace = new WorkspaceWindow(primaryStage, null);
        
        // Add toolbar to workspace
        ToolBar toolbar = createToolbar();
        BorderPane root = workspace.getRoot();
        VBox topContainer = new VBox();
        topContainer.getChildren().addAll(root.getTop(), toolbar);
        root.setTop(topContainer);
        
        // Create and set scene
        Scene scene = workspace.createScene();
        primaryStage.setScene(scene);
        primaryStage.show();
        
        // Start update loop for status bar
        startUpdateLoop();
        
        // Log initial status
        workspace.getConsolePane().info("Application started");
        workspace.getConsolePane().info("Ready - Native engine not loaded (build C++ library first)");
        workspace.getConsolePane().info("Click 'Initialize Engine' to start the engine");
    }
    
    private ToolBar createToolbar() {
        ToolBar toolbar = new ToolBar();
        
        Button initButton = new Button("Initialize Engine");
        initButton.setOnAction(e -> initializeEngine());
        
        Button testButton = new Button("Create Entity");
        testButton.setOnAction(e -> createTestEntity());
        
        Button test1000Button = new Button("Create 1000");
        test1000Button.setOnAction(e -> createManyEntities(1000));
        
        Button test50kButton = new Button("Create 50k");
        test50kButton.setOnAction(e -> createManyEntities(50000));
        
        Button clearButton = new Button("Clear All");
        clearButton.setOnAction(e -> clearScene());
        
        Separator separator = new Separator();
        
        Button aboutButton = new Button("About");
        aboutButton.setOnAction(e -> showAbout());
        
        toolbar.getItems().addAll(
            initButton,
            testButton,
            test1000Button,
            test50kButton,
            clearButton,
            separator,
            aboutButton
        );
        
        return toolbar;
    }
    
    /**
     * Start update loop for status bar and telemetry.
     */
    private void startUpdateLoop() {
        updateTimer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                // Throttle status updates (10 Hz)
                if (now - lastStatusUpdate > STATUS_UPDATE_INTERVAL_NS) {
                    workspace.updateStatus();
                    
                    // Update telemetry if engine is valid
                    if (engine != null && engine.isValid() && engine.isTelemetryEnabled()) {
                        workspace.updateTelemetry();
                    }
                    
                    lastStatusUpdate = now;
                }
            }
        };
        updateTimer.start();
    }
    
    private void initializeEngine() {
        try {
            if (engine != null) {
                workspace.getConsolePane().warning("Engine already initialized");
                return;
            }
            
            workspace.getConsolePane().info("Initializing engine...");
            engine = new NativeEngine(1280, 720, true);
            
            // Update workspace with engine reference
            workspace.setEngine(engine);
            
            workspace.getConsolePane().info("Engine initialized successfully");
            workspace.getConsolePane().info("Resolution: 1280x720");
            workspace.getConsolePane().info("Telemetry: " + (engine.isTelemetryEnabled() ? "Enabled" : "Disabled"));
            
            // Note: In a full implementation, you would now:
            // 1. Create a viewport tab with the engine
            // 2. Connect scene inspector to viewport picking
            // 3. Enable telemetry pane updates
            workspace.getConsolePane().info("Tip: Add a viewport to the center tab area");
            
        } catch (Exception e) {
            workspace.getConsolePane().error("Failed to initialize engine", e);
            showError("Engine Initialization Failed", e.getMessage());
        }
    }
    
    private void createTestEntity() {
        if (engine == null || !engine.isValid()) {
            workspace.getConsolePane().warning("Engine not initialized");
            return;
        }
        
        if (workspace.getSceneManager() == null) {
            workspace.getConsolePane().warning("Scene manager not available");
            return;
        }
        
        try {
            var entity = workspace.getSceneManager().createEntity();
            
            // Set random position
            entity.setPosX(Math.random() * 20 - 10);
            entity.setPosY(Math.random() * 20 - 10);
            entity.setPosZ(Math.random() * 20 - 10);
            
            // Set random color
            entity.setColorR(Math.random());
            entity.setColorG(Math.random());
            entity.setColorB(Math.random());
            
            // Sync to engine
            workspace.getSceneManager().syncTransformToEngine(entity);
            workspace.getSceneManager().syncColorToEngine(entity);
            
            workspace.getConsolePane().info("Created entity: " + entity.getEntityId());
        } catch (Exception e) {
            workspace.getConsolePane().error("Failed to create entity", e);
        }
    }
    
    private void createManyEntities(int count) {
        if (engine == null || !engine.isValid()) {
            workspace.getConsolePane().warning("Engine not initialized");
            return;
        }
        
        if (workspace.getSceneManager() == null) {
            workspace.getConsolePane().warning("Scene manager not available");
            return;
        }
        
        try {
            workspace.getConsolePane().info("Creating " + count + " entities...");
            long startTime = System.currentTimeMillis();
            
            for (int i = 0; i < count; i++) {
                var entity = workspace.getSceneManager().createEntity();
                
                // Set position in a grid pattern for large counts
                if (count > 100) {
                    int gridSize = (int) Math.ceil(Math.cbrt(count));
                    int x = i % gridSize;
                    int y = (i / gridSize) % gridSize;
                    int z = i / (gridSize * gridSize);
                    
                    entity.setPosX(x * 2.0);
                    entity.setPosY(y * 2.0);
                    entity.setPosZ(z * 2.0);
                } else {
                    // Random positions for small counts
                    entity.setPosX(Math.random() * 20 - 10);
                    entity.setPosY(Math.random() * 20 - 10);
                    entity.setPosZ(Math.random() * 20 - 10);
                }
                
                // Set color gradient
                float hue = (float) i / count;
                entity.setColorR(Math.sin(hue * Math.PI));
                entity.setColorG(Math.cos(hue * Math.PI));
                entity.setColorB(Math.sin(hue * Math.PI * 2));
                
                // Sync every 100 entities to avoid too many individual calls
                if (i % 100 == 0 || i == count - 1) {
                    workspace.getSceneManager().syncTransformToEngine(entity);
                    workspace.getSceneManager().syncColorToEngine(entity);
                }
            }
            
            long elapsed = System.currentTimeMillis() - startTime;
            workspace.getConsolePane().info(
                String.format("Created %d entities in %d ms (%.1f entities/sec)",
                    count, elapsed, count * 1000.0 / elapsed));
            
            // Refresh outliner
            if (workspace.getSceneOutlinerPane() != null) {
                workspace.getSceneOutlinerPane().refresh();
            }
            
        } catch (Exception e) {
            workspace.getConsolePane().error("Failed to create entities", e);
        }
    }
    
    private void clearScene() {
        if (workspace.getSceneManager() != null) {
            workspace.getConsolePane().info("Clearing all entities...");
            workspace.getSceneManager().clearAll();
            workspace.getConsolePane().info("Scene cleared");
            
            // Refresh outliner
            if (workspace.getSceneOutlinerPane() != null) {
                workspace.getSceneOutlinerPane().refresh();
            }
        } else if (engine != null) {
            workspace.getConsolePane().info("Closing engine...");
            engine.close();
            engine = null;
            workspace.getConsolePane().info("Engine closed");
        } else {
            workspace.getConsolePane().warning("Nothing to clear");
        }
    }
    
    private void showAbout() {
        // Use workspace's about dialog
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
    
    private void showError(String title, String message) {
        Alert alert = new Alert(Alert.AlertType.ERROR);
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(message);
        alert.showAndWait();
    }
    
    @Override
    public void stop() {
        // Stop update loop
        if (updateTimer != null) {
            updateTimer.stop();
        }
        
        // Save workspace layout
        if (workspace != null) {
            workspace.saveLayout();
        }
        
        // Close engine
        if (engine != null) {
            workspace.getConsolePane().info("Shutting down engine...");
            engine.close();
        }
        
        System.out.println("[AstraeusApp] Application stopped");
    }
    
    public static void main(String[] args) {
        launch(args);
    }

    static class Starter {
        public static void main(String[] args) {
            AstraeusApp.main(args);
        }
    }
}
