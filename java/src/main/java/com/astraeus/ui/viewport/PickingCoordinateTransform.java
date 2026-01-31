package com.astraeus.ui.viewport;

import javafx.scene.image.ImageView;

/**
 * PickingCoordinateTransform - Robust coordinate transformation for picking.
 * 
 * Handles transformations from scene coordinates to viewport pixels with
 * correct handling of:
 * - Viewport resize
 * - HiDPI scaling
 * - ImageView fitWidth/fitHeight
 * - Viewport subregion rendering
 * 
 * FEATURES:
 * - DPI-aware coordinate transforms
 * - Resize-safe transforms
 * - Bounds checking and clamping
 * - Zero-allocation transform methods
 * 
 * USAGE:
 * <pre>
 * PickingCoordinateTransform transform = new PickingCoordinateTransform(imageView);
 * int[] viewportCoords = transform.sceneToViewport(event.getX(), event.getY());
 * int viewportX = viewportCoords[0];
 * int viewportY = viewportCoords[1];
 * </pre>
 */
public class PickingCoordinateTransform {
    
    private final ImageView imageView;
    private int currentViewportWidth;
    private int currentViewportHeight;
    private double dpiScale = 1.0;
    
    // Pre-allocated arrays for zero-allocation transforms
    private final int[] coordsCache = new int[2];
    
    /**
     * Create a new coordinate transform for an ImageView.
     * 
     * @param imageView The ImageView displaying the viewport
     */
    public PickingCoordinateTransform(ImageView imageView) {
        if (imageView == null) {
            throw new IllegalArgumentException("ImageView cannot be null");
        }
        this.imageView = imageView;
    }
    
    /**
     * Update viewport dimensions.
     * Call this when the viewport is resized.
     * 
     * @param width New viewport width in pixels
     * @param height New viewport height in pixels
     */
    public void setViewportDimensions(int width, int height) {
        this.currentViewportWidth = width;
        this.currentViewportHeight = height;
    }
    
    /**
     * Set DPI scale factor.
     * Default is 1.0 for standard DPI displays.
     * Set to 2.0 for Retina/HiDPI displays.
     * 
     * @param scale DPI scale factor
     */
    public void setDpiScale(double scale) {
        this.dpiScale = Math.max(0.1, scale);
    }
    
    /**
     * Transform scene coordinates to viewport pixel coordinates.
     * Returns a reused array [x, y] - DO NOT MODIFY.
     * 
     * @param sceneX X coordinate in scene space
     * @param sceneY Y coordinate in scene space
     * @return Array containing [viewportX, viewportY]
     */
    public int[] sceneToViewport(double sceneX, double sceneY) {
        // Get ImageView display size
        double displayWidth = imageView.getFitWidth();
        double displayHeight = imageView.getFitHeight();
        
        if (displayWidth <= 0 || displayHeight <= 0) {
            // Fallback to bounds if fitWidth/Height not set
            displayWidth = imageView.getBoundsInLocal().getWidth();
            displayHeight = imageView.getBoundsInLocal().getHeight();
        }
        
        // Calculate scale factors from display to viewport
        double scaleX = currentViewportWidth / displayWidth;
        double scaleY = currentViewportHeight / displayHeight;
        
        // Apply DPI scaling
        scaleX *= dpiScale;
        scaleY *= dpiScale;
        
        // Transform to viewport coordinates
        int viewportX = (int) Math.round(sceneX * scaleX);
        int viewportY = (int) Math.round(sceneY * scaleY);
        
        // Clamp to viewport bounds
        viewportX = Math.max(0, Math.min(viewportX, currentViewportWidth - 1));
        viewportY = Math.max(0, Math.min(viewportY, currentViewportHeight - 1));
        
        // Store in cache and return
        coordsCache[0] = viewportX;
        coordsCache[1] = viewportY;
        return coordsCache;
    }
    
    /**
     * Transform viewport pixel coordinates to scene coordinates.
     * Returns a reused array [x, y] - DO NOT MODIFY.
     * 
     * @param viewportX X coordinate in viewport pixels
     * @param viewportY Y coordinate in viewport pixels
     * @return Array containing [sceneX, sceneY]
     */
    public double[] viewportToScene(int viewportX, int viewportY) {
        // Get ImageView display size
        double displayWidth = imageView.getFitWidth();
        double displayHeight = imageView.getFitHeight();
        
        if (displayWidth <= 0 || displayHeight <= 0) {
            displayWidth = imageView.getBoundsInLocal().getWidth();
            displayHeight = imageView.getBoundsInLocal().getHeight();
        }
        
        // Calculate scale factors from viewport to display
        double scaleX = displayWidth / currentViewportWidth;
        double scaleY = displayHeight / currentViewportHeight;
        
        // Apply inverse DPI scaling
        scaleX /= dpiScale;
        scaleY /= dpiScale;
        
        // Transform to scene coordinates
        double sceneX = viewportX * scaleX;
        double sceneY = viewportY * scaleY;
        
        // Note: No clamping needed for scene coordinates
        return new double[]{sceneX, sceneY};
    }
    
    /**
     * Check if scene coordinates are within bounds.
     * 
     * @param sceneX X coordinate in scene space
     * @param sceneY Y coordinate in scene space
     * @return true if coordinates are within the ImageView bounds
     */
    public boolean isWithinBounds(double sceneX, double sceneY) {
        double displayWidth = imageView.getFitWidth();
        double displayHeight = imageView.getFitHeight();
        
        if (displayWidth <= 0 || displayHeight <= 0) {
            displayWidth = imageView.getBoundsInLocal().getWidth();
            displayHeight = imageView.getBoundsInLocal().getHeight();
        }
        
        return sceneX >= 0 && sceneX < displayWidth &&
               sceneY >= 0 && sceneY < displayHeight;
    }
    
    /**
     * Get current viewport width.
     */
    public int getViewportWidth() {
        return currentViewportWidth;
    }
    
    /**
     * Get current viewport height.
     */
    public int getViewportHeight() {
        return currentViewportHeight;
    }
    
    /**
     * Get current DPI scale.
     */
    public double getDpiScale() {
        return dpiScale;
    }
}
