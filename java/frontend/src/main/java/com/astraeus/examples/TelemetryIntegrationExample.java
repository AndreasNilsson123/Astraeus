package com.astraeus.examples;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.model.FrameStats;
import com.astraeus.tools.TelemetryOverlay;
import com.astraeus.tools.TelemetryPane;
import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.layout.*;
import javafx.stage.Stage;

/**
 * Example integration of telemetry UI components with Astraeus engine.
 * 
 * Demonstrates:
 * 1. TelemetryOverlay - lightweight HUD on viewport
 * 2. TelemetryPane - detailed panel in side dock
 * 3. Throttled updates (~30 Hz) to minimize UI overhead
 * 4. Toggle visibility controls
 * 
 * INTEGRATION PATTERN:
 * - Main viewport uses StackPane with overlay on top
 * - Side panel uses TelemetryPane in BorderPane
 * - AnimationTimer drives updates with throttling
 * - Telemetry can be toggled on/off at runtime
 */
public class TelemetryIntegrationExample extends Application {

    static class Starter {
        public static void main(String[] args) {
            TelemetryIntegrationExample.main(args);
        }
    }
    
    private NativeEngine engine;
    private TelemetryOverlay overlay;
    private TelemetryPane telemetryPane;
    
    // Update throttling (30 Hz = ~33ms between updates)
    private static final long TELEMETRY_UPDATE_INTERVAL_NS = 33_000_000L; // 33ms
    private long lastTelemetryUpdate = 0;
    
    @Override
    public void start(Stage primaryStage) {
        // Initialize engine
        engine = new NativeEngine(1280, 720, false);
        engine.configureReadback(1920, 1080, true);
        engine.enableTelemetry(true); // Enable telemetry by default
        
        // === VIEWPORT WITH OVERLAY ===
        
        // Placeholder for actual viewport (replace with AstraeusViewport)
        Pane viewportPlaceholder = new Pane();
        viewportPlaceholder.setStyle("-fx-background-color: #1a1a1a;");
        viewportPlaceholder.setPrefSize(1280, 720);
        
        // Create telemetry overlay
        overlay = new TelemetryOverlay();
        overlay.setVisible(true);
        
        // Stack overlay on top of viewport
        StackPane viewportStack = new StackPane();
        viewportStack.getChildren().addAll(viewportPlaceholder, overlay);
        StackPane.setAlignment(overlay, Pos.TOP_RIGHT);
        StackPane.setMargin(overlay, new javafx.geometry.Insets(10));
        
        // === SIDE PANEL ===
        
        telemetryPane = new TelemetryPane(engine);
        
        // Control buttons
        VBox controls = new VBox(10);
        controls.setPadding(new javafx.geometry.Insets(10));
        
        Button toggleOverlayBtn = new Button("Toggle HUD Overlay");
        toggleOverlayBtn.setOnAction(e -> overlay.toggleVisible());
        
        Button toggleTelemetryBtn = new Button("Toggle Telemetry");
        toggleTelemetryBtn.setOnAction(e -> {
            boolean enabled = !engine.isTelemetryEnabled();
            engine.enableTelemetry(enabled);
            System.out.println("Telemetry " + (enabled ? "enabled" : "disabled"));
        });
        
        controls.getChildren().addAll(toggleOverlayBtn, toggleTelemetryBtn);
        
        VBox rightPanel = new VBox();
        rightPanel.getChildren().addAll(controls, telemetryPane);
        VBox.setVgrow(telemetryPane, Priority.ALWAYS);
        
        // === MAIN LAYOUT ===
        
        BorderPane root = new BorderPane();
        root.setCenter(viewportStack);
        root.setRight(rightPanel);
        
        // === RENDER LOOP ===
        
        AnimationTimer renderLoop = new AnimationTimer() {
            private long lastFrameTime = 0;
            
            @Override
            public void handle(long now) {
                if (lastFrameTime == 0) {
                    lastFrameTime = now;
                    return;
                }
                
                // Calculate delta time
                double deltaTime = (now - lastFrameTime) / 1_000_000_000.0;
                lastFrameTime = now;
                
                // Render frame
                engine.beginFrame(deltaTime);
                // ... render scene here ...
                engine.endFrame();
                
                // Update telemetry UI (throttled to ~30 Hz)
                if (engine.isTelemetryEnabled() && 
                    (now - lastTelemetryUpdate) >= TELEMETRY_UPDATE_INTERVAL_NS) {
                    
                    updateTelemetryUI();
                    lastTelemetryUpdate = now;
                }
            }
        };
        
        // === WINDOW SETUP ===
        
        Scene scene = new Scene(root, 1600, 900);
        primaryStage.setTitle("Astraeus Telemetry Integration Example");
        primaryStage.setScene(scene);
        primaryStage.setOnCloseRequest(e -> {
            renderLoop.stop();
            engine.close();
        });
        
        primaryStage.show();
        renderLoop.start();
    }
    
    /**
     * Update telemetry UI components.
     * Called at ~30 Hz to minimize overhead.
     */
    private void updateTelemetryUI() {
        try {
            // Get current stats
            FrameStats stats = engine.getTelemetryStats();
            
            // Update overlay
            if (overlay.isVisible()) {
                overlay.update(stats);
            }
            
            // Update detailed panel
            telemetryPane.update();
            
        } catch (Exception e) {
            System.err.println("[TelemetryUI] Update error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        launch(args);
    }
}

/**
 * INTEGRATION GUIDE FOR EXISTING APPLICATIONS:
 * 
 * 1. ADD OVERLAY TO VIEWPORT:
 * 
 *    // In your viewport setup (e.g., AstraeusApp.java):
 *    TelemetryOverlay overlay = new TelemetryOverlay();
 *    
 *    StackPane viewportStack = new StackPane();
 *    viewportStack.getChildren().addAll(yourViewport, overlay);
 *    StackPane.setAlignment(overlay, Pos.TOP_RIGHT);
 *    StackPane.setMargin(overlay, new Insets(10));
 *    
 *    // Toggle with keyboard shortcut:
 *    scene.setOnKeyPressed(e -> {
 *        if (e.getCode() == KeyCode.F1) {
 *            overlay.toggleVisible();
 *        }
 *    });
 * 
 * 
 * 2. ADD DETAILED PANEL TO TOOLING WINDOW:
 * 
 *    // In your tools window:
 *    TelemetryPane telemetryPane = new TelemetryPane(engine);
 *    toolsTabPane.getTabs().add(new Tab("Telemetry", telemetryPane));
 * 
 * 
 * 3. UPDATE IN RENDER LOOP (WITH THROTTLING):
 * 
 *    private static final long TELEMETRY_UPDATE_NS = 33_000_000L; // 30 Hz
 *    private long lastTelemetryUpdate = 0;
 *    
 *    // In your AnimationTimer.handle():
 *    if (engine.isTelemetryEnabled() && 
 *        (now - lastTelemetryUpdate) >= TELEMETRY_UPDATE_NS) {
 *        
 *        TelemetryFrameStats stats = engine.getTelemetryStats();
 *        overlay.update(stats);
 *        telemetryPane.update();
 *        
 *        lastTelemetryUpdate = now;
 *    }
 * 
 * 
 * 4. ENABLE/DISABLE TELEMETRY:
 * 
 *    // Enable at startup (or on-demand):
 *    engine.enableTelemetry(true);
 *    
 *    // Toggle with button:
 *    toggleButton.setOnAction(e -> {
 *        engine.enableTelemetry(!engine.isTelemetryEnabled());
 *    });
 * 
 * 
 * 5. PERFORMANCE TIPS:
 * 
 *    - Always throttle UI updates to ~30 Hz (not every frame)
 *    - Check isTelemetryEnabled() before updating UI
 *    - When telemetry is disabled, there is ZERO native overhead
 *    - Overlay and Pane reuse existing UI nodes (no per-frame allocations)
 *    - Use overlay.setVisible(false) when not needed to skip updates
 */
