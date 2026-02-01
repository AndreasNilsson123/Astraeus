package com.astraeus.rendering;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.model.PickResult;
import javafx.scene.layout.StackPane;

import java.util.function.Consumer;

/**
 * FxViewportSurface - Binds a native surface/viewport to JavaFX presentation.
 * 
 * This is a thin wrapper around FxViewport that provides a cleaner API
 * for the ViewportPane to interact with.
 * 
 * SAFETY:
 * - Wraps native color buffer with stable PixelBuffer (no reallocations)
 * - Viewport-only resize (backing buffer remains fixed)
 * - Zero-copy frame presentation
 * 
 * FEATURES:
 * - Camera control (orbit/fly/pan)
 * - Entity picking
 * - Overlay stack (HUD, selection, telemetry)
 */
public class FxViewportSurface extends StackPane {
    
    private final FxViewport viewport;
    
    /**
     * Create a new viewport surface.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum backing buffer width
     * @param maxHeight Maximum backing buffer height
     * @param initialWidth Initial viewport width
     * @param initialHeight Initial viewport height
     */
    public FxViewportSurface(NativeEngine engine, int maxWidth, int maxHeight,
                            int initialWidth, int initialHeight) {
        // Create underlying viewport
        this.viewport = new FxViewport(engine, maxWidth, maxHeight, 
                                        initialWidth, initialHeight);
        
        // Add to this pane
        getChildren().add(viewport);
        
        // Bind size
        viewport.prefWidthProperty().bind(widthProperty());
        viewport.prefHeightProperty().bind(heightProperty());
        viewport.minWidthProperty().bind(widthProperty());
        viewport.minHeightProperty().bind(heightProperty());
        viewport.maxWidthProperty().bind(widthProperty());
        viewport.maxHeightProperty().bind(heightProperty());
        
        System.out.println("[FxViewportSurface] Created surface wrapper");
    }
    
    /**
     * Update viewport state.
     * Call this every frame before rendering.
     * 
     * @param deltaTime Time since last update (seconds)
     */
    public void update(double deltaTime) {
        viewport.update(deltaTime);
    }
    
    /**
     * Update display with latest engine output.
     * Call this after rendering each frame.
     */
    public void updateDisplay() {
        viewport.updateDisplay();
    }
    
    /**
     * Resize the viewport (viewport region only, not backing buffer).
     * 
     * @param width New viewport width
     * @param height New viewport height
     */
    public void resizeViewport(int width, int height) {
        viewport.resizeViewport(width, height);
    }
    
    /**
     * Set entity selection callback.
     */
    public void setOnEntitySelected(Consumer<PickResult> callback) {
        viewport.setOnEntitySelected(callback);
    }
    
    /**
     * Get viewport controller for camera manipulation.
     */
    public ViewportController getController() {
        return viewport.getController();
    }
    
    /**
     * Get overlay stack for adding custom overlays.
     */
    public OverlayStack getOverlayStack() {
        return viewport.getOverlayStack();
    }
    
    /**
     * Get native engine reference.
     */
    public NativeEngine getEngine() {
        return viewport.getEngine();
    }
    
    /**
     * Get selected entity ID.
     */
    public int getSelectedEntityId() {
        return viewport.getSelectedEntityId();
    }
    
    /**
     * Clear selection.
     */
    public void clearSelection() {
        viewport.clearSelection();
    }
    
    /**
     * Enable/disable input handling.
     */
    public void setInputEnabled(boolean enabled) {
        viewport.setInputEnabled(enabled);
    }
    
    /**
     * Check if input is enabled.
     */
    public boolean isInputEnabled() {
        return viewport.isInputEnabled();
    }
    
    /**
     * Get current viewport width.
     */
    public int getCurrentWidth() {
        return viewport.getCurrentWidth();
    }
    
    /**
     * Get current viewport height.
     */
    public int getCurrentHeight() {
        return viewport.getCurrentHeight();
    }
}
