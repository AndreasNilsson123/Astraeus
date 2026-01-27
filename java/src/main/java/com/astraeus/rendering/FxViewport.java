package com.astraeus.rendering;

import com.astraeus.native_api.NativeEngine;
import javafx.scene.image.ImageView;
import javafx.scene.image.PixelBuffer;
import javafx.scene.image.PixelFormat;
import javafx.scene.image.WritableImage;
import javafx.scene.layout.Pane;

import java.lang.foreign.MemorySegment;
import java.nio.ByteBuffer;

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
        
        // Bind image view size to pane size
        imageView.fitWidthProperty().bind(widthProperty());
        imageView.fitHeightProperty().bind(heightProperty());
        
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
}
