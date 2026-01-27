package com.astraeus.test;

import com.astraeus.native_api.FrameStatsView;
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
import javafx.scene.paint.Color;
import javafx.stage.Stage;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;
import java.lang.invoke.VarHandle;

/**
 * Standalone telemetry UI test without native library dependency.
 * Uses mock frame stats to demonstrate telemetry components.
 * 
 * This is useful for:
 * - UI development and testing
 * - Verifying no per-frame allocations
 * - Testing telemetry update logic
 */
public class TelemetryUIStandaloneTest extends Application {
    
    private TelemetryOverlay overlay;
    private TelemetryPane telemetryPane;
    
    private AnimationTimer renderLoop;
    private long lastFrameTime = 0;
    private long lastTelemetryUpdate = 0;
    private static final long TELEMETRY_UPDATE_INTERVAL_NS = 33_333_333; // ~30 Hz
    
    private int frameCount = 0;
    private int entityCount = 0;
    
    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Astraeus - Telemetry UI Standalone Test");
        
        // Create viewport placeholder
        Pane viewportPlaceholder = new Pane();
        viewportPlaceholder.setStyle("-fx-background-color: #333333;");
        viewportPlaceholder.setPrefSize(800, 600);
        
        Label placeholderLabel = new Label("Viewport Placeholder\n(Native library not loaded)");
        placeholderLabel.setTextFill(Color.WHITE);
        placeholderLabel.setStyle("-fx-font-size: 18; -fx-font-weight: bold;");
        StackPane.setAlignment(placeholderLabel, Pos.CENTER);
        
        // Create telemetry overlay
        overlay = new TelemetryOverlay();
        
        // Stack viewport and overlay
        StackPane viewportStack = new StackPane();
        viewportStack.getChildren().addAll(viewportPlaceholder, placeholderLabel, overlay);
        StackPane.setAlignment(overlay, Pos.TOP_LEFT);
        StackPane.setMargin(overlay, new Insets(10));
        
        // Create telemetry pane
        telemetryPane = new TelemetryPane();
        
        // Create layout
        BorderPane root = new BorderPane();
        
        // Top toolbar
        ToolBar toolbar = createToolbar();
        root.setTop(toolbar);
        
        // Center: viewport with overlay
        root.setCenter(viewportStack);
        
        // Right panel: telemetry
        VBox rightPanel = new VBox(10);
        rightPanel.setPadding(new Insets(10));
        rightPanel.getChildren().add(telemetryPane);
        ScrollPane rightScroll = new ScrollPane(rightPanel);
        rightScroll.setFitToWidth(true);
        rightScroll.setPrefWidth(380);
        root.setRight(rightScroll);
        
        // Bottom status bar
        Label statusLabel = new Label("Press F3 to toggle telemetry overlay | ESC to exit | Mock data mode");
        statusLabel.setStyle("-fx-padding: 5; -fx-background-color: #f0f0f0;");
        root.setBottom(statusLabel);
        
        // Create scene
        Scene scene = new Scene(root, 1400, 800);
        
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
        
        // Start mock render loop
        startMockRenderLoop();
    }
    
    private ToolBar createToolbar() {
        ToolBar toolbar = new ToolBar();
        
        Button addEntityButton = new Button("Add Entity (Mock)");
        addEntityButton.setOnAction(e -> entityCount++);
        
        Button add10Button = new Button("Add 10 Entities");
        add10Button.setOnAction(e -> entityCount += 10);
        
        Separator separator = new Separator();
        
        CheckBox overlayToggle = new CheckBox("Overlay (F3)");
        overlayToggle.setSelected(overlay.isEnabled());
        overlayToggle.setOnAction(e -> overlay.setEnabled(overlayToggle.isSelected()));
        
        CheckBox panelToggle = new CheckBox("Panel");
        panelToggle.setSelected(telemetryPane.isEnabled());
        panelToggle.setOnAction(e -> telemetryPane.setEnabled(panelToggle.isSelected()));
        
        Label infoLabel = new Label("Mock Data Mode");
        infoLabel.setStyle("-fx-text-fill: orange; -fx-font-weight: bold;");
        
        toolbar.getItems().addAll(
            addEntityButton,
            add10Button,
            separator,
            new Label("Telemetry:"),
            overlayToggle,
            panelToggle,
            separator,
            infoLabel
        );
        
        return toolbar;
    }
    
    private void startMockRenderLoop() {
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
                frameCount++;
                
                // Update telemetry at controlled rate (~30 Hz)
                if (now - lastTelemetryUpdate >= TELEMETRY_UPDATE_INTERVAL_NS) {
                    updateTelemetry(deltaTime);
                    lastTelemetryUpdate = now;
                }
            }
        };
        
        renderLoop.start();
        System.out.println("[TelemetryUITest] Mock render loop started");
    }
    
    private void updateTelemetry(double deltaTime) {
        // Create mock frame stats
        FrameStatsView stats = createMockStats(frameCount, deltaTime);
        
        // Update both overlay and panel (no allocations)
        overlay.update(stats);
        telemetryPane.update(stats);
    }
    
    /**
     * Create mock frame stats for UI testing.
     */
    private FrameStatsView createMockStats(long frameNumber, double deltaTime) {
        try (Arena arena = Arena.ofConfined()) {
            // Allocate memory for mock FrameStats
            MemorySegment statsStruct = arena.allocate(com.astraeus.native_api.EngineBindings.FRAME_STATS_LAYOUT);
            
            // Get var handles
            var layout = com.astraeus.native_api.EngineBindings.FRAME_STATS_LAYOUT;
            VarHandle frameNumberHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("frame_number"));
            VarHandle deltaTimeMsHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("delta_time_ms"));
            VarHandle renderTimeMsHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("render_time_ms"));
            VarHandle drawCallsHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("draw_calls"));
            VarHandle triangleCountHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("triangle_count"));
            VarHandle entityCountHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("entity_count"));
            
            // Fill with mock data
            double deltaMs = deltaTime * 1000.0;
            double renderMs = deltaMs * 0.6 + Math.random() * 2.0; // Simulate render time
            int drawCalls = 10 + entityCount * 2;
            int triangles = 100 + entityCount * 20;
            
            frameNumberHandle.set(statsStruct, 0L, frameNumber);
            deltaTimeMsHandle.set(statsStruct, 0L, deltaMs);
            renderTimeMsHandle.set(statsStruct, 0L, renderMs);
            drawCallsHandle.set(statsStruct, 0L, drawCalls);
            triangleCountHandle.set(statsStruct, 0L, triangles);
            entityCountHandle.set(statsStruct, 0L, entityCount);
            
            // Create FrameStatsView from struct
            return new FrameStatsView(statsStruct);
        }
    }
    
    @Override
    public void stop() {
        if (renderLoop != null) {
            renderLoop.stop();
        }
        System.out.println("[TelemetryUITest] Shutdown complete");
    }
    
    public static void main(String[] args) {
        launch(args);
    }
}
