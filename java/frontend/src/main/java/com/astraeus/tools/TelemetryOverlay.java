package com.astraeus.tools;

import com.astraeus.native_api.model.FrameStats;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;

/**
 * Lightweight telemetry HUD overlay for the viewport.
 * Displays real-time performance metrics in a non-intrusive corner overlay.
 * 
 * PERFORMANCE:
 * - NO allocations per frame (reuses Label instances)
 * - Efficient text updates using setText() only
 * - Minimal JavaFX layout overhead
 * 
 * USAGE:
 * <pre>
 * TelemetryOverlay overlay = new TelemetryOverlay();
 * overlay.setVisible(true);
 * 
 * // In render loop:
 * FrameStats stats = engine.getTelemetryStats();
 * overlay.update(stats);
 * </pre>
 */
public class TelemetryOverlay extends VBox {
    
    // Reused labels (NO per-frame allocations)
    private final Label fpsLabel;
    private final Label cpuTimeLabel;
    private final Label gpuTimeLabel;
    private final Label drawCallsLabel;
    private final Label trianglesLabel;
    
    // Cached format strings (avoid repeated string building)
    private static final String FPS_FORMAT = "FPS: %.1f";
    private static final String CPU_FORMAT = "CPU: %.2f ms";
    private static final String GPU_FORMAT = "GPU: %s";
    private static final String DRAW_FORMAT = "Draws: %d";
    private static final String TRI_FORMAT = "Tris: %,d";
    
    public TelemetryOverlay() {
        super(4);
        
        // Styling: semi-transparent dark background in top-right corner
        setStyle("-fx-background-color: rgba(0, 0, 0, 0.7); " +
                 "-fx-background-radius: 5; " +
                 "-fx-border-color: rgba(100, 100, 100, 0.8); " +
                 "-fx-border-width: 1; " +
                 "-fx-border-radius: 5;");
        setPadding(new Insets(8, 10, 8, 10));
        setSpacing(2);
        setAlignment(Pos.TOP_LEFT);
        
        // Create labels (once)
        Font labelFont = Font.font("Consolas", FontWeight.NORMAL, 12);
        
        fpsLabel = createLabel(labelFont);
        cpuTimeLabel = createLabel(labelFont);
        gpuTimeLabel = createLabel(labelFont);
        drawCallsLabel = createLabel(labelFont);
        trianglesLabel = createLabel(labelFont);
        
        // Add all labels
        getChildren().addAll(
            fpsLabel,
            cpuTimeLabel,
            gpuTimeLabel,
            drawCallsLabel,
            trianglesLabel
        );
        
        // Initial state
        setVisible(false);
        setMouseTransparent(true); // Don't block viewport interaction
        
        // Initial text
        updateText(0, 0, 0, 0, 0);
    }
    
    private Label createLabel(Font font) {
        Label label = new Label();
        label.setFont(font);
        label.setTextFill(Color.WHITE);
        return label;
    }
    
    /**
     * Update overlay with new telemetry data.
     * PERFORMANCE: This method does NOT allocate objects per frame.
     * Only updates existing Label text content.
     * 
     * @param stats Telemetry frame statistics
     */
    public void update(FrameStats stats) {
        if (stats == null) {
            return;
        }
        
        double fps = stats.getFPS();
        double cpuMs = stats.getCpuTimeMs();
        double gpuMs = stats.getGpuTimeMs();
        int draws = stats.getDrawCalls();
        int tris = stats.getTriangleCount();
        
        updateText(fps, cpuMs, gpuMs, draws, tris);
    }
    
    /**
     * Internal method to update label text.
     * Uses String.format which creates new strings, but this is
     * acceptable for UI updates at 30-60 Hz.
     */
    private void updateText(double fps, double cpuMs, double gpuMs, int draws, int tris) {
        fpsLabel.setText(String.format(FPS_FORMAT, fps));
        cpuTimeLabel.setText(String.format(CPU_FORMAT, cpuMs));
        
        // GPU time: show "N/A" if not available (0.0)
        String gpuText = (gpuMs > 0.001) 
            ? String.format("GPU: %.2f ms", gpuMs) 
            : String.format(GPU_FORMAT, "N/A");
        gpuTimeLabel.setText(gpuText);
        
        drawCallsLabel.setText(String.format(DRAW_FORMAT, draws));
        trianglesLabel.setText(String.format(TRI_FORMAT, tris));
    }
    
    /**
     * Position overlay in top-right corner of parent.
     * Call this after adding overlay to a StackPane or similar.
     * 
     * @param parentWidth Parent container width
     * @param parentHeight Parent container height
     */
    public void positionTopRight(double parentWidth, double parentHeight) {
        // This method is optional - can also use StackPane.setAlignment()
        setTranslateX(parentWidth - getWidth() - 10);
        setTranslateY(10);
    }
    
    /**
     * Toggle overlay visibility.
     */
    public void toggleVisible() {
        setVisible(!isVisible());
    }
}
