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
        
        Button clearButton = new Button("Clear");
        clearButton.setOnAction(e -> clearScene());
        
        Separator separator = new Separator();
        
        Button aboutButton = new Button("About");
        aboutButton.setOnAction(e -> showAbout());
        
        toolbar.getItems().addAll(
            initButton,
            testButton,
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
        
        try {
            int entityId = engine.createEntity();
            workspace.getConsolePane().info("Created entity: " + entityId);
        } catch (Exception e) {
            workspace.getConsolePane().error("Failed to create entity", e);
        }
    }
    
    private void clearScene() {
        if (engine != null) {
            workspace.getConsolePane().info("Closing engine...");
            engine.close();
            engine = null;
            workspace.getConsolePane().info("Engine closed");
        } else {
            workspace.getConsolePane().warning("No engine to close");
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
