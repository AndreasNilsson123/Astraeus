package com.astraeus.tools;

import com.astraeus.native_api.FrameStatsView;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

/**
 * On-viewport HUD overlay for displaying real-time telemetry.
 * Shows FPS, CPU time, GPU time, draw calls, and triangle count.
 * 
 * Design goals:
 * - Minimal, non-intrusive overlay
 * - No per-frame allocations (reuses labels)
 * - Updates at controlled rate (caller responsibility)
 * - Toggle visibility
 */
public class TelemetryOverlay extends VBox {
    
    private final Label fpsLabel;
    private final Label cpuTimeLabel;
    private final Label gpuTimeLabel;
    private final Label drawCallsLabel;
    private final Label trianglesLabel;
    private final Label entitiesLabel;
    
    private boolean enabled = true;
    
    public TelemetryOverlay() {
        super(2);
        
        // Styling: Semi-transparent dark background
        setBackground(new Background(new BackgroundFill(
            new Color(0, 0, 0, 0.7),
            new CornerRadii(5),
            Insets.EMPTY
        )));
        setPadding(new Insets(8, 12, 8, 12));
        setAlignment(Pos.TOP_LEFT);
        setMouseTransparent(true);  // Don't block viewport interaction
        
        // Create labels (reused every frame, no allocations)
        fpsLabel = createLabel("FPS: --");
        cpuTimeLabel = createLabel("CPU: -- ms");
        gpuTimeLabel = createLabel("GPU: -- ms");
        drawCallsLabel = createLabel("Draws: --");
        trianglesLabel = createLabel("Tris: --");
        entitiesLabel = createLabel("Entities: --");
        
        getChildren().addAll(
            fpsLabel,
            cpuTimeLabel,
            gpuTimeLabel,
            drawCallsLabel,
            trianglesLabel,
            entitiesLabel
        );
        
        // Set initial size and position
        setPrefWidth(180);
        setMaxWidth(180);
    }
    
    /**
     * Create a styled label for the overlay.
     */
    private Label createLabel(String text) {
        Label label = new Label(text);
        label.setTextFill(Color.WHITE);
        label.setStyle("-fx-font-family: 'Monospaced', monospace; -fx-font-size: 12px;");
        return label;
    }
    
    /**
     * Update the overlay with new frame statistics.
     * This method reuses existing labels, causing no allocations.
     * 
     * @param stats Current frame statistics
     */
    public void update(FrameStatsView stats) {
        if (!enabled || !isVisible()) {
            return;
        }
        
        // Update labels in-place (no allocations)
        fpsLabel.setText(String.format("FPS: %.1f", stats.getFPS()));
        cpuTimeLabel.setText(String.format("CPU: %.2f ms", stats.getDeltaTimeMs()));
        gpuTimeLabel.setText(String.format("GPU: %.2f ms", stats.getRenderTimeMs()));
        drawCallsLabel.setText(String.format("Draws: %,d", stats.getDrawCalls()));
        trianglesLabel.setText(String.format("Tris: %,d", stats.getTriangleCount()));
        entitiesLabel.setText(String.format("Entities: %,d", stats.getEntityCount()));
    }
    
    /**
     * Toggle overlay visibility.
     */
    public void toggle() {
        setEnabled(!enabled);
    }
    
    /**
     * Enable or disable the overlay.
     * When disabled, overlay is hidden and updates are skipped.
     */
    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
        setVisible(enabled);
    }
    
    /**
     * Check if overlay is enabled.
     */
    public boolean isEnabled() {
        return enabled;
    }
}
