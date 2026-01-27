package com.astraeus.test;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.FrameStatsView;
import com.astraeus.rendering.FxViewport;
import com.astraeus.tools.TelemetryOverlay;
import com.astraeus.tools.TelemetryPane;
import com.astraeus.tools.SceneInspector;
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
 * Demo application showing the telemetry system in action.
 * 
 * Features:
 * - TelemetryOverlay (on-viewport HUD)
 * - TelemetryPane (detailed panel)
 * - Toggle with F3 key
 * - Controlled update rate (30 Hz for telemetry)
 */
public class TelemetryDemoApp extends Application {
    
    private NativeEngine engine;
    private FxViewport viewport;
    private TelemetryOverlay overlay;
    private TelemetryPane telemetryPane;
    private SceneInspector sceneInspector;
    
    private AnimationTimer renderLoop;
    private long lastFrameTime = 0;
    private long lastTelemetryUpdate = 0;
    private static final long TELEMETRY_UPDATE_INTERVAL_NS = 33_333_333; // ~30 Hz
    
    private int entityCount = 0;
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - Telemetry Demo");
        
        try {
            // Initialize native engine
            int maxWidth = 2560;
            int maxHeight = 1440;
            int initialWidth = 1280;
            int initialHeight = 720;
            
            engine = new NativeEngine(initialWidth, initialHeight, false);
            System.out.println("[TelemetryDemo] Engine initialized");
            
            // Create viewport with telemetry overlay
            viewport = new FxViewport(engine, maxWidth, maxHeight, initialWidth, initialHeight);
            
            // Create telemetry overlay (on-viewport HUD)
            overlay = new TelemetryOverlay();
            
            // Position overlay at top-left of viewport
            StackPane viewportStack = new StackPane();
            viewportStack.getChildren().addAll(viewport, overlay);
            StackPane.setAlignment(overlay, Pos.TOP_LEFT);
            StackPane.setMargin(overlay, new Insets(10));
            
            // Create telemetry pane (detailed panel)
            telemetryPane = new TelemetryPane();
            
            // Create scene inspector
            sceneInspector = new SceneInspector();
            viewport.setOnEntitySelected(sceneInspector::updateSelection);
            
            // Create layout
            BorderPane root = new BorderPane();
            
            // Top toolbar
            ToolBar toolbar = createToolbar();
            root.setTop(toolbar);
            
            // Center: viewport with overlay
            root.setCenter(viewportStack);
            
            // Right panel: telemetry + scene inspector
            VBox rightPanel = new VBox(10);
            rightPanel.setPadding(new Insets(10));
            rightPanel.getChildren().addAll(telemetryPane, sceneInspector);
            ScrollPane rightScroll = new ScrollPane(rightPanel);
            rightScroll.setFitToWidth(true);
            rightScroll.setPrefWidth(380);
            root.setRight(rightScroll);
            
            // Bottom status bar
            Label statusLabel = new Label("Press F3 to toggle telemetry overlay | ESC to exit");
            statusLabel.setStyle("-fx-padding: 5; -fx-background-color: #f0f0f0;");
            root.setBottom(statusLabel);
            
            // Create scene
            Scene scene = new Scene(root, 1680, 900);
            
            // Keyboard shortcuts
            scene.addEventFilter(KeyEvent.KEY_PRESSED, event -> {
                if (event.getCode() == KeyCode.F3) {
                    overlay.toggle();
                    event.consume();
                } else if (event.getCode() == KeyCode.ESCAPE) {
                    primaryStage.close();
                    event.consume();
                }
            });
            
            primaryStage.setScene(scene);
            primaryStage.show();
            
            // Start render loop
            startRenderLoop();
            
        } catch (Exception e) {
            e.printStackTrace();
            showError("Failed to initialize", e.getMessage());
            primaryStage.close();
        }
    }
    
    private ToolBar createToolbar() {
        ToolBar toolbar = new ToolBar();
        
        Button addEntityButton = new Button("Add Entity");
        addEntityButton.setOnAction(e -> addTestEntity());
        
        Button add10Button = new Button("Add 10 Entities");
        add10Button.setOnAction(e -> {
            for (int i = 0; i < 10; i++) {
                addTestEntity();
            }
        });
        
        Button add100Button = new Button("Add 100 Entities");
        add100Button.setOnAction(e -> {
            for (int i = 0; i < 100; i++) {
                addTestEntity();
            }
        });
        
        Separator separator = new Separator();
        
        CheckBox overlayToggle = new CheckBox("Overlay (F3)");
        overlayToggle.setSelected(overlay.isEnabled());
        overlayToggle.setOnAction(e -> overlay.setEnabled(overlayToggle.isSelected()));
        
        CheckBox panelToggle = new CheckBox("Panel");
        panelToggle.setSelected(telemetryPane.isEnabled());
        panelToggle.setOnAction(e -> telemetryPane.setEnabled(panelToggle.isSelected()));
        
        toolbar.getItems().addAll(
            addEntityButton,
            add10Button,
            add100Button,
            separator,
            new Label("Telemetry:"),
            overlayToggle,
            panelToggle
        );
        
        return toolbar;
    }
    
    private void addTestEntity() {
        if (engine == null || !engine.isValid()) {
            return;
        }
        
        int entityId = engine.createEntity();
        entityCount++;
        
        // Random position
        float x = (float) (Math.random() * 20 - 10);
        float y = (float) (Math.random() * 20 - 10);
        float z = (float) (Math.random() * 20 - 10);
        
        System.out.println("[TelemetryDemo] Created entity " + entityId + 
                         " at (" + x + ", " + y + ", " + z + ")");
    }
    
    private void startRenderLoop() {
        renderLoop = new AnimationTimer() {
            @Override
            public void handle(long now) {
                if (lastFrameTime == 0) {
                    lastFrameTime = now;
                    lastTelemetryUpdate = now;
                    return;
                }
                
                // Calculate delta time
                double deltaTime = (now - lastFrameTime) / 1_000_000_000.0;
                lastFrameTime = now;
                
                try {
                    // Render frame
                    engine.beginFrame(deltaTime);
                    engine.endFrame();
                    
                    // Update viewport display
                    viewport.updateDisplay();
                    
                    // Update telemetry at controlled rate (~30 Hz)
                    if (now - lastTelemetryUpdate >= TELEMETRY_UPDATE_INTERVAL_NS) {
                        updateTelemetry();
                        lastTelemetryUpdate = now;
                    }
                    
                } catch (Exception e) {
                    System.err.println("[TelemetryDemo] Render error: " + e.getMessage());
                    e.printStackTrace();
                    stop();
                }
            }
        };
        
        renderLoop.start();
        System.out.println("[TelemetryDemo] Render loop started");
    }
    
    private void updateTelemetry() {
        try {
            // Get frame stats from engine
            FrameStatsView stats = engine.getFrameStats();
            
            // Update both overlay and panel (no allocations)
            overlay.update(stats);
            telemetryPane.update(stats);
            
        } catch (Exception e) {
            System.err.println("[TelemetryDemo] Telemetry update error: " + e.getMessage());
        }
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
        if (renderLoop != null) {
            renderLoop.stop();
        }
        if (engine != null) {
            engine.close();
        }
        System.out.println("[TelemetryDemo] Shutdown complete");
    }
    
    public static void main(String[] args) {
        launch(args);
    }
}
