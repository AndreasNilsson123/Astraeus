package com.astraeus.tools;

import com.astraeus.native_api.FrameStatsView;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;

/**
 * Compact HUD-style overlay for displaying real-time telemetry.
 * Designed to be positioned over the viewport with minimal visual intrusion.
 * 
 * Features:
 * - Real-time FPS display
 * - CPU frame time (render_time_ms)
 * - GPU frame time (gpu_time_ms)
 * - Draw calls and triangle count
 * - Semi-transparent dark background
 * - Compact layout with monospace font
 * 
 * Usage:
 * <pre>
 *   TelemetryOverlay overlay = new TelemetryOverlay();
 *   overlay.setVisible(true);
 *   
 *   // Each frame:
 *   engine.getFrameStats(statsView);
 *   overlay.update(statsView);
 * </pre>
 */
public class TelemetryOverlay extends VBox {
    
    private final Label fpsLabel;
    private final Label cpuLabel;
    private final Label gpuLabel;
    private final Label drawCallsLabel;
    private final Label trianglesLabel;
    
    private static final String LABEL_STYLE = 
        "-fx-text-fill: white; " +
        "-fx-font-family: 'Courier New', monospace; " +
        "-fx-font-size: 12px;";
    
    /**
     * Create a new telemetry overlay.
     * Initially visible; call setVisible(false) to hide.
     */
    public TelemetryOverlay() {
        super(2);  // 2px spacing between labels
        
        // Container styling - semi-transparent dark background
        setStyle(
            "-fx-background-color: rgba(0, 0, 0, 0.75); " +
            "-fx-background-radius: 5; " +
            "-fx-border-color: rgba(255, 255, 255, 0.2); " +
            "-fx-border-width: 1; " +
            "-fx-border-radius: 5;"
        );
        setPadding(new Insets(8, 12, 8, 12));
        setAlignment(Pos.TOP_LEFT);
        setPickOnBounds(false);  // Allow clicks to pass through to viewport
        setMouseTransparent(true);  // Overlay doesn't capture mouse events
        
        // Create labels
        fpsLabel = createLabel("FPS: --");
        cpuLabel = createLabel("CPU: --");
        gpuLabel = createLabel("GPU: --");
        drawCallsLabel = createLabel("Draws: --");
        trianglesLabel = createLabel("Tris: --");
        
        // Add to container
        getChildren().addAll(fpsLabel, cpuLabel, gpuLabel, drawCallsLabel, trianglesLabel);
        
        // Set initial size constraints
        setMinWidth(USE_PREF_SIZE);
        setMaxWidth(USE_PREF_SIZE);
        setPrefWidth(140);
    }
    
    /**
     * Create a styled label for telemetry data.
     */
    private Label createLabel(String text) {
        Label label = new Label(text);
        label.setStyle(LABEL_STYLE);
        label.setFont(Font.font("Courier New", FontWeight.NORMAL, 12));
        return label;
    }
    
    /**
     * Update overlay with latest frame statistics.
     * This method is allocation-free and can be called every frame.
     * 
     * @param stats Frame statistics view (reusable)
     */
    public void update(FrameStatsView stats) {
        if (!isVisible()) {
            return;  // Skip update if not visible
        }
        
        // Update labels with formatted values
        fpsLabel.setText(String.format("FPS: %.1f", stats.getFPS()));
        cpuLabel.setText(String.format("CPU: %.2fms", stats.getRenderTimeMs()));
        gpuLabel.setText(String.format("GPU: %.2fms", stats.getGpuTimeMs()));
        drawCallsLabel.setText(String.format("Draws: %d", stats.getDrawCalls()));
        trianglesLabel.setText(String.format("Tris: %s", formatCount(stats.getTriangleCount())));
    }
    
    /**
     * Format large counts with K/M suffixes.
     * Examples: 1234 -> "1.2K", 1234567 -> "1.2M"
     */
    private String formatCount(int count) {
        if (count < 1000) {
            return String.valueOf(count);
        } else if (count < 1000000) {
            return String.format("%.1fK", count / 1000.0);
        } else {
            return String.format("%.1fM", count / 1000000.0);
        }
    }
    
    /**
     * Clear all displayed values (show placeholders).
     */
    public void clear() {
        fpsLabel.setText("FPS: --");
        cpuLabel.setText("CPU: --");
        gpuLabel.setText("GPU: --");
        drawCallsLabel.setText("Draws: --");
        trianglesLabel.setText("Tris: --");
    }
    
    /**
     * Set corner position style.
     * Convenience method to position in viewport corners.
     * 
     * @param corner One of: "top-left", "top-right", "bottom-left", "bottom-right"
     */
    public void setCornerPosition(String corner) {
        // This is a hint for the parent layout; actual positioning
        // should be done via StackPane.setAlignment() or AnchorPane constraints
        switch (corner.toLowerCase()) {
            case "top-left":
                setAlignment(Pos.TOP_LEFT);
                break;
            case "top-right":
                setAlignment(Pos.TOP_RIGHT);
                break;
            case "bottom-left":
                setAlignment(Pos.BOTTOM_LEFT);
                break;
            case "bottom-right":
                setAlignment(Pos.BOTTOM_RIGHT);
                break;
            default:
                System.err.println("[TelemetryOverlay] Unknown corner: " + corner);
        }
    }
}
