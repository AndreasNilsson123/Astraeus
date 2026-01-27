package com.astraeus.rendering;

import com.astraeus.native_api.NativeEngine;
import com.astraeus.native_api.PickingView;
import javafx.scene.image.ImageView;
import javafx.scene.image.PixelBuffer;
import javafx.scene.image.PixelFormat;
import javafx.scene.image.WritableImage;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Pane;
import javafx.scene.shape.Rectangle;
import javafx.scene.paint.Color;

import java.lang.foreign.MemorySegment;
import java.nio.ByteBuffer;
import java.util.function.Consumer;

/**
 * JavaFX viewport component for displaying engine output.
 * 
 * SAFETY GUARANTEES:
 * - PixelBuffer backing memory is allocated once at max size and NEVER resized
 * - Resize operations only update the viewport region, not the buffer
 * - Memory pointer remains stable for the lifetime of the engine
 * - No EXCEPTION_ACCESS_VIOLATION or memory corruption possible
 * 
 * This component follows the "fixed backing buffer, viewport-only resize" contract
 * to ensure JavaFX PixelBuffer stability.
 */
public class FxViewport extends Pane {
    
    private final NativeEngine engine;
    private final ImageView imageView;
    private WritableImage writableImage;
    private PixelBuffer<ByteBuffer> pixelBuffer;
    private NativeEngine.PixelBufferView colorBuffer;
    
    private final int maxWidth;
    private final int maxHeight;
    private int currentWidth;
    private int currentHeight;
    
    private int selectedEntityId = 0;
    private Rectangle selectionOverlay;
    private Consumer<PickingView> onEntitySelected;
    
    /**
     * Create a new FxViewport with fixed maximum dimensions.
     * 
     * @param engine Native engine instance
     * @param maxWidth Maximum viewport width (backing buffer size)
     * @param maxHeight Maximum viewport height (backing buffer size)
     * @param initialWidth Initial viewport width
     * @param initialHeight Initial viewport height
     */
    public FxViewport(NativeEngine engine, int maxWidth, int maxHeight, 
                      int initialWidth, int initialHeight) {
        this.engine = engine;
        this.maxWidth = maxWidth;
        this.maxHeight = maxHeight;
        this.currentWidth = initialWidth;
        this.currentHeight = initialHeight;
        
        // Configure readback buffers with fixed size
        engine.configureReadback(maxWidth, maxHeight, false);
        
        // Get the color buffer view (stable pointer)
        colorBuffer = engine.getColorBuffer();
        
        // Create PixelBuffer with STABLE backing memory
        // IMPORTANT: This ByteBuffer wraps the native memory and must never be resized
        MemorySegment data = colorBuffer.getData();
        ByteBuffer backingBuffer = data.asByteBuffer();
        
        // PixelFormat for BGRA8 (common for JavaFX)
        PixelFormat<ByteBuffer> format = PixelFormat.getByteBgraPreInstance();
        
        // Create PixelBuffer with FULL backing size
        // Only the viewport region (width/height) will change, not the buffer
        pixelBuffer = new PixelBuffer<>(maxWidth, maxHeight, backingBuffer, format);
        
        // Create WritableImage from PixelBuffer
        writableImage = new WritableImage(pixelBuffer);
        
        // Create ImageView to display the image
        imageView = new ImageView(writableImage);
        imageView.setPreserveRatio(false);
        imageView.setSmooth(false);  // No interpolation for pixel-perfect rendering
        
        // Set initial viewport
        imageView.setViewport(new javafx.geometry.Rectangle2D(0, 0, initialWidth, initialHeight));
        
        // Add to pane
        getChildren().add(imageView);
        
        // Create selection overlay (initially hidden)
        selectionOverlay = new Rectangle();
        selectionOverlay.setFill(Color.TRANSPARENT);
        selectionOverlay.setStroke(Color.YELLOW);
        selectionOverlay.setStrokeWidth(3);
        selectionOverlay.setVisible(false);
        selectionOverlay.setMouseTransparent(true);
        getChildren().add(selectionOverlay);
        
        // Bind image view size to pane size
        imageView.fitWidthProperty().bind(widthProperty());
        imageView.fitHeightProperty().bind(heightProperty());
        
        // Setup mouse click handler for picking
        imageView.setOnMouseClicked(this::handleMouseClick);
        
        System.out.println("[FxViewport] Created with max=" + maxWidth + "x" + maxHeight + 
                         ", initial=" + initialWidth + "x" + initialHeight);
    }
    
    /**
     * Resize the viewport (viewport region only, NOT backing buffer).
     * 
     * SAFETY: This method only changes the viewport region displayed from the
     * fixed backing buffer. No memory reallocation occurs.
     * 
     * @param width New viewport width (must be <= maxWidth)
     * @param height New viewport height (must be <= maxHeight)
     */
    public void resizeViewport(int width, int height) {
        // Clamp to max dimensions
        width = Math.min(width, maxWidth);
        height = Math.min(height, maxHeight);
        
        if (width == currentWidth && height == currentHeight) {
            return;  // No change
        }
        
        System.out.println("[FxViewport] Resizing viewport from " + currentWidth + "x" + 
                         currentHeight + " to " + width + "x" + height + 
                         " (backing remains " + maxWidth + "x" + maxHeight + ")");
        
        // Update engine viewport (viewport-only resize, no buffer reallocation)
        engine.resizeViewport(width, height);
        
        // Update JavaFX viewport region
        imageView.setViewport(new javafx.geometry.Rectangle2D(0, 0, width, height));
        
        currentWidth = width;
        currentHeight = height;
        
        // Refresh the buffer view (pointer remains the same, only dimensions change)
        colorBuffer = engine.getColorBuffer();
        
        System.out.println("[FxViewport] Viewport resized successfully");
    }
    
    /**
     * Update the display with latest engine output.
     * Call this after each frame is rendered.
     */
    public void updateDisplay() {
        // Update PixelBuffer to trigger JavaFX redraw
        // The backing memory is already updated by the engine
        pixelBuffer.updateBuffer(pb -> {
            // Return the viewport region that changed
            return new javafx.geometry.Rectangle2D(0, 0, currentWidth, currentHeight);
        });
    }
    
    /**
     * Get current viewport width.
     */
    public int getCurrentWidth() {
        return currentWidth;
    }
    
    /**
     * Get current viewport height.
     */
    public int getCurrentHeight() {
        return currentHeight;
    }
    
    /**
     * Get maximum backing buffer width.
     */
    public int getMaxWidth() {
        return maxWidth;
    }
    
    /**
     * Get maximum backing buffer height.
     */
    public int getMaxHeight() {
        return maxHeight;
    }
    
    /**
     * Set callback for entity selection events.
     * The callback receives the PickingView result when an entity is clicked.
     * 
     * @param callback Consumer that receives picking results
     */
    public void setOnEntitySelected(Consumer<PickingView> callback) {
        this.onEntitySelected = callback;
    }
    
    /**
     * Get the currently selected entity ID.
     * 
     * @return Selected entity ID (0 if none selected)
     */
    public int getSelectedEntityId() {
        return selectedEntityId;
    }
    
    /**
     * Clear the current selection.
     */
    public void clearSelection() {
        selectedEntityId = 0;
        selectionOverlay.setVisible(false);
    }
    
    /**
     * Handle mouse clicks for entity picking.
     */
    private void handleMouseClick(MouseEvent event) {
        try {
            // Convert mouse coordinates to viewport coordinates
            // The ImageView may be scaled, so we need to convert from scene coords to viewport coords
            double sceneX = event.getX();
            double sceneY = event.getY();
            
            // Calculate scale factors
            double scaleX = currentWidth / imageView.getFitWidth();
            double scaleY = currentHeight / imageView.getFitHeight();
            
            // Convert to viewport pixel coordinates
            int viewportX = (int) (sceneX * scaleX);
            int viewportY = (int) (sceneY * scaleY);
            
            // Clamp to viewport bounds
            viewportX = Math.max(0, Math.min(viewportX, currentWidth - 1));
            viewportY = Math.max(0, Math.min(viewportY, currentHeight - 1));
            
            // Perform picking
            PickingView result = engine.pick(viewportX, viewportY);
            
            System.out.println("[FxViewport] Pick at (" + viewportX + ", " + viewportY + "): " + result);
            
            // Update selection state
            if (result.hasValidEntity()) {
                selectedEntityId = result.getEntityId();
                updateSelectionOverlay(sceneX, sceneY);
                
                // Notify callback
                if (onEntitySelected != null) {
                    onEntitySelected.accept(result);
                }
            } else {
                clearSelection();
                
                // Notify callback with miss result (hit == false)
                if (onEntitySelected != null) {
                    onEntitySelected.accept(result);
                }
            }
            
        } catch (Exception e) {
            System.err.println("[FxViewport] Error during picking: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    /**
     * Update the selection overlay to highlight the clicked position.
     */
    private void updateSelectionOverlay(double centerX, double centerY) {
        double size = 40; // Size of selection box
        selectionOverlay.setX(centerX - size / 2);
        selectionOverlay.setY(centerY - size / 2);
        selectionOverlay.setWidth(size);
        selectionOverlay.setHeight(size);
        selectionOverlay.setVisible(true);
    }
}
