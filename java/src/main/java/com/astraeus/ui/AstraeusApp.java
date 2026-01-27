package com.astraeus.ui;

import com.astraeus.native_api.NativeEngine;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

/**
 * Main JavaFX application for Astraeus visualization engine.
 */
public class AstraeusApp extends Application {
    
    private NativeEngine engine;
    private Label statusLabel;
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - 3D Visualization Engine");
        
        // Create UI
        BorderPane root = new BorderPane();
        
        // Top toolbar
        ToolBar toolbar = createToolbar();
        root.setTop(toolbar);
        
        // Center viewport (placeholder for now)
        VBox centerPane = new VBox(10);
        centerPane.setStyle("-fx-padding: 20; -fx-alignment: center;");
        Label titleLabel = new Label("Astraeus 3D Visualization Engine");
        titleLabel.setStyle("-fx-font-size: 24; -fx-font-weight: bold;");
        statusLabel = new Label("Engine not initialized");
        statusLabel.setStyle("-fx-font-size: 14;");
        centerPane.getChildren().addAll(titleLabel, statusLabel);
        root.setCenter(centerPane);
        
        // Bottom status bar
        HBox statusBar = new HBox(10);
        statusBar.setStyle("-fx-padding: 5; -fx-background-color: #f0f0f0;");
        Label versionLabel = new Label("Version 0.1.0");
        statusBar.getChildren().add(versionLabel);
        root.setBottom(statusBar);
        
        // Create scene
        Scene scene = new Scene(root, 1280, 720);
        primaryStage.setScene(scene);
        primaryStage.show();
        
        // Note: Engine initialization would happen here
        // For now, it's a stub since native library needs to be built first
        updateStatus("Ready - Native engine not loaded (build C++ library first)");
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
    
    private void initializeEngine() {
        try {
            if (engine != null) {
                updateStatus("Engine already initialized");
                return;
            }
            
            engine = new NativeEngine(1280, 720, true);
            updateStatus("Engine initialized successfully");
            
        } catch (Exception e) {
            updateStatus("Failed to initialize: " + e.getMessage());
            showError("Engine Initialization Failed", e.getMessage());
        }
    }
    
    private void createTestEntity() {
        if (engine == null || !engine.isValid()) {
            updateStatus("Engine not initialized");
            return;
        }
        
        try {
            int entityId = engine.createEntity();
            updateStatus("Created entity: " + entityId);
        } catch (Exception e) {
            updateStatus("Failed to create entity: " + e.getMessage());
        }
    }
    
    private void clearScene() {
        if (engine != null) {
            engine.close();
            engine = null;
            updateStatus("Engine closed");
        }
    }
    
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
    
    private void showError(String title, String message) {
        Alert alert = new Alert(Alert.AlertType.ERROR);
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(message);
        alert.showAndWait();
    }
    
    private void updateStatus(String status) {
        if (statusLabel != null) {
            statusLabel.setText(status);
        }
        System.out.println("[AstraeusApp] " + status);
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
}
