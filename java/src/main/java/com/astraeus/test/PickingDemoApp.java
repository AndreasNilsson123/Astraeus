package com.astraeus.test;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.PickingView;
import com.astraeus.rendering.FxViewport;
import com.astraeus.tools.SceneInspector;
import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;

/**
 * Picking demonstration application.
 * 
 * Demonstrates entity picking via ID buffer:
 * - Click entities in the viewport to select them
 * - Selected entity is highlighted with yellow outline
 * - Entity metadata is displayed in the inspector panel
 * - Picking works with viewport resizing and camera movement
 * 
 * ACCEPTANCE CRITERIA:
 * - Clicking entity selects it reliably
 * - Selection overlay appears at clicked location
 * - Inspector shows entity metadata
 * - Works under resizing and camera movement
 */
public class PickingDemoApp extends Application {
    
    private NativeEngine engine;
    private FxViewport viewport;
    private SceneInspector inspector;
    private Label statusLabel;
    private Label fpsLabel;
    private Label pickInfoLabel;
    
    private long startTime;
    private long lastFrameTime;
    private int frameCount = 0;
    private double fps = 0.0;
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - Picking Demo");
        
        try {
            // Initialize engine
            engine = new NativeEngine(1280, 720, true);
            
            // Create FxViewport with large max dimensions
            viewport = new FxViewport(engine, 2560, 1440, 1280, 720);
            viewport.setPrefSize(1280, 720);
            
            // Create scene inspector
            inspector = new SceneInspector();
            
            // Setup picking callback
            viewport.setOnEntitySelected(this::handleEntitySelected);
            
            // Create UI
            BorderPane root = new BorderPane();
            
            // Top toolbar
            ToolBar toolbar = createToolbar();
            root.setTop(toolbar);
            
            // Center: viewport
            root.setCenter(viewport);
            
            // Right: inspector panel
            root.setRight(inspector);
            
            // Bottom status bar
            VBox statusArea = createStatusArea();
            root.setBottom(statusArea);
            
            // Create scene
            Scene scene = new Scene(root, 1600, 900);
            primaryStage.setScene(scene);
            primaryStage.show();
            
            // Start render loop
            startRenderLoop();
            
            updateStatus("Ready - Click entities in the viewport to select them");
            
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
        
        Button resize800Button = new Button("Resize 800x600");
        resize800Button.setOnAction(e -> resizeViewport(800, 600));
        
        Button resize1080Button = new Button("Resize 1920x1080");
        resize1080Button.setOnAction(e -> resizeViewport(1920, 1080));
        
        Separator separator = new Separator();
        
        Button aboutButton = new Button("About");
        aboutButton.setOnAction(e -> showAbout());
        
        toolbar.getItems().addAll(
            createEntityButton,
            clearSelectionButton,
            new Separator(),
            resize800Button,
            resize1080Button,
            separator,
            aboutButton
        );
        
        return toolbar;
    }
    
    private VBox createStatusArea() {
        VBox statusArea = new VBox(5);
        statusArea.setPadding(new Insets(5));
        statusArea.setStyle("-fx-background-color: #f0f0f0;");
        
        HBox statusBar = new HBox(20);
        
        statusLabel = new Label("Ready");
        fpsLabel = new Label("FPS: 0.0");
        pickInfoLabel = new Label("No entity selected");
        
        statusBar.getChildren().addAll(
            new Label("Status:"),
            statusLabel,
            new Separator(),
            fpsLabel,
            new Separator(),
            pickInfoLabel
        );
        
        Label instructions = new Label(
            "Instructions: Click any entity in the viewport to select it. " +
            "The selected entity will be highlighted and its metadata will be shown in the inspector panel."
        );
        instructions.setWrapText(true);
        instructions.setStyle("-fx-font-size: 11; -fx-text-fill: #666666;");
        
        statusArea.getChildren().addAll(statusBar, instructions);
        
        return statusArea;
    }
    
    private void startRenderLoop() {
        startTime = System.nanoTime();
        lastFrameTime = startTime;
        
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
                    
                    // Update FPS counter
                    frameCount++;
                    double elapsed = (now - startTime) / 1_000_000_000.0;
                    if (elapsed >= 1.0) {
                        fps = frameCount / elapsed;
                        fpsLabel.setText(String.format("FPS: %.1f", fps));
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
    
    private void handleEntitySelected(PickingView pickResult) {
        // Update inspector with pick result
        inspector.updateSelection(pickResult);
        
        // Update status
        if (pickResult.hasValidEntity()) {
            pickInfoLabel.setText(String.format("Selected: Entity #%d at (%.2f, %.2f, %.2f)",
                                              pickResult.getEntityId(),
                                              pickResult.getWorldX(),
                                              pickResult.getWorldY(),
                                              pickResult.getWorldZ()));
            updateStatus("Entity selected - see inspector for details");
        } else {
            pickInfoLabel.setText("No entity selected");
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
        pickInfoLabel.setText("No entity selected");
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
        alert.setTitle("About Astraeus Picking Demo");
        alert.setHeaderText("Astraeus 3D Visualization Engine");
        alert.setContentText(
            "Picking Demonstration Application\n\n" +
            "This application demonstrates entity picking via ID buffer:\n" +
            "- Click entities in the viewport to select them\n" +
            "- Selected entity is highlighted with yellow outline\n" +
            "- Entity metadata is displayed in the inspector panel\n" +
            "- Picking works with viewport resizing and camera movement\n\n" +
            "Version: 0.1.0\n" +
            "Task: B2 - Picking via ID buffer"
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
            PickingDemoApp.main(args);
        }
    }
}
