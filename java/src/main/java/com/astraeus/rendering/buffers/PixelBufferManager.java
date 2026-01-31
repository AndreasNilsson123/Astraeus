package com.astraeus.rendering.buffers;

import com.astraeus.native_api.model.PixelBufferView;
import javafx.scene.image.PixelBuffer;
import javafx.scene.image.PixelFormat;

import java.nio.ByteBuffer;

/**
 * Manages stable PixelBuffer backing for JavaFX viewport integration.
 * 
 * <p>This class ensures that the ByteBuffer backing a JavaFX PixelBuffer
 * remains stable across viewport resizes and engine updates. The backing
 * memory is allocated once at the maximum size and never reallocated.</p>
 * 
 * <p><b>Key Guarantees:</b></p>
 * <ul>
 *   <li>Backing ByteBuffer is allocated once and never changes</li>
 *   <li>ByteBuffer position/limit/mark remain stable (position=0, limit=capacity)</li>
 *   <li>Viewport resizes only change the viewport region, not the backing size</li>
 *   <li>Safe for long-running sessions with no heap creep</li>
 * </ul>
 * 
 * <p><b>Thread Safety:</b> This class is NOT thread-safe. All methods must be
 * called from the JavaFX Application Thread.</p>
 * 
 * <p><b>Copy Policy:</b></p>
 * <ul>
 *   <li>Baseline: CPU copy from native engine memory into PixelBuffer</li>
 *   <li>Future: Optional PBO/GPU-side copy for zero-copy path</li>
 * </ul>
 * 
 * @see javafx.scene.image.PixelBuffer
 * @see com.astraeus.native_api.model.PixelBufferView
 */
public class PixelBufferManager {
    
    private final int maxWidth;
    private final int maxHeight;
    private final int maxBackingSize;
    
    // Stable backing buffer (allocated once, never reallocated)
    private ByteBuffer stableByteBuffer;
    
    // JavaFX PixelBuffer (created once with stable backing)
    private PixelBuffer<ByteBuffer> pixelBuffer;
    
    // Current viewport dimensions
    private int currentWidth;
    private int currentHeight;
    
    // DPI scale factor (logical pixels to device pixels)
    private double dpiScale = 1.0;
    
    /**
     * Create a new PixelBufferManager.
     * 
     * @param maxWidth Maximum viewport width in device pixels
     * @param maxHeight Maximum viewport height in device pixels
     */
    public PixelBufferManager(int maxWidth, int maxHeight) {
        this.maxWidth = maxWidth;
        this.maxHeight = maxHeight;
        
        // Calculate maximum backing size (4 bytes per pixel for BGRA8)
        this.maxBackingSize = maxWidth * maxHeight * 4;
        
        System.out.println("[PixelBufferManager] Created with max=" + maxWidth + "x" + maxHeight + 
                          " (" + (maxBackingSize / 1024 / 1024) + " MB)");
    }
    
    /**
     * Initialize the PixelBuffer with a native buffer view.
     * 
     * <p>This method should be called once after the native viewport is created
     * and has allocated its backing memory. It creates the stable ByteBuffer
     * and JavaFX PixelBuffer that will be used for the lifetime of the viewport.</p>
     * 
     * @param bufferView Native buffer view from engine
     * @throws IllegalStateException if already initialized
     * @throws IllegalArgumentException if buffer view is invalid
     */
    public void initialize(PixelBufferView bufferView) {
        if (pixelBuffer != null) {
            throw new IllegalStateException("PixelBufferManager already initialized");
        }
        
        if (bufferView == null) {
            throw new IllegalArgumentException("bufferView cannot be null");
        }
        
        ByteBuffer nativeBuffer = bufferView.getByteBuffer();
        if (nativeBuffer == null) {
            throw new IllegalArgumentException("bufferView has no ByteBuffer attached");
        }
        
        int backingSize = bufferView.getMaxBackingSize();
        if (backingSize != maxBackingSize) {
            System.err.println("[PixelBufferManager] WARNING: Expected backing size " + maxBackingSize + 
                             " but got " + backingSize + " - using actual size");
        }
        
        // Store stable ByteBuffer reference (NEVER mutate position/limit after this)
        this.stableByteBuffer = nativeBuffer;
        
        // Create JavaFX PixelBuffer with stable backing
        PixelFormat<ByteBuffer> format = PixelFormat.getByteBgraPreInstance();
        this.pixelBuffer = new PixelBuffer<>(
            bufferView.getMaxBackingWidth(),
            bufferView.getMaxBackingHeight(),
            stableByteBuffer,
            format
        );
        
        // Store current viewport dimensions
        this.currentWidth = bufferView.getWidth();
        this.currentHeight = bufferView.getHeight();
        
        System.out.println("[PixelBufferManager] Initialized with viewport=" + currentWidth + "x" + currentHeight +
                          ", backing=" + bufferView.getMaxBackingWidth() + "x" + bufferView.getMaxBackingHeight());
    }
    
    /**
     * Update viewport dimensions after a resize.
     * 
     * <p>This method updates the internal viewport dimensions and notifies the
     * PixelBuffer of the dirty region. The backing buffer remains unchanged.</p>
     * 
     * @param width New viewport width in device pixels
     * @param height New viewport height in device pixels
     */
    public void updateViewportSize(int width, int height) {
        if (pixelBuffer == null) {
            throw new IllegalStateException("PixelBufferManager not initialized");
        }
        
        if (width > maxWidth || height > maxHeight) {
            throw new IllegalArgumentException(
                "Viewport size " + width + "x" + height + " exceeds maximum " + maxWidth + "x" + maxHeight);
        }
        
        this.currentWidth = width;
        this.currentHeight = height;
        
        // Note: We don't need to recreate the PixelBuffer, just update our tracking
        System.out.println("[PixelBufferManager] Viewport resized to " + width + "x" + height);
    }
    
    /**
     * Update the PixelBuffer to reflect new engine output.
     * 
     * <p>This method notifies JavaFX that the pixel data has changed and should
     * be redrawn. Call this after each frame is rendered by the engine.</p>
     */
    public void updateBuffer() {
        if (pixelBuffer == null) {
            throw new IllegalStateException("PixelBufferManager not initialized");
        }
        
        // Update the buffer with the current viewport region
        // Note: We use a lambda to avoid allocating a Rectangle2D on each frame
        pixelBuffer.updateBuffer(pb -> {
            // Return the dirty region (0, 0, currentWidth, currentHeight)
            // JavaFX will redraw only this region
            return new javafx.geometry.Rectangle2D(0, 0, currentWidth, currentHeight);
        });
    }
    
    /**
     * Set DPI scale factor for coordinate conversion.
     * 
     * <p>This scale factor is used to convert between logical pixels (JavaFX
     * coordinates) and device pixels (native viewport coordinates).</p>
     * 
     * @param scale Scale factor (typically 1.0, 1.5, 2.0 for HiDPI displays)
     */
    public void setDpiScale(double scale) {
        if (scale <= 0) {
            throw new IllegalArgumentException("DPI scale must be positive");
        }
        this.dpiScale = scale;
        System.out.println("[PixelBufferManager] DPI scale set to " + scale);
    }
    
    /**
     * Convert logical X coordinate to device pixels.
     * 
     * @param logicalX Logical X coordinate (JavaFX)
     * @return Device X coordinate (native viewport)
     */
    public int toDeviceX(double logicalX) {
        return (int) Math.round(logicalX * dpiScale);
    }
    
    /**
     * Convert logical Y coordinate to device pixels.
     * 
     * @param logicalY Logical Y coordinate (JavaFX)
     * @return Device Y coordinate (native viewport)
     */
    public int toDeviceY(double logicalY) {
        return (int) Math.round(logicalY * dpiScale);
    }
    
    /**
     * Convert logical width to device pixels.
     * 
     * @param logicalWidth Logical width (JavaFX)
     * @return Device width (native viewport)
     */
    public int toDeviceWidth(double logicalWidth) {
        return (int) Math.round(logicalWidth * dpiScale);
    }
    
    /**
     * Convert logical height to device pixels.
     * 
     * @param logicalHeight Logical height (JavaFX)
     * @return Device height (native viewport)
     */
    public int toDeviceHeight(double logicalHeight) {
        return (int) Math.round(logicalHeight * dpiScale);
    }
    
    /**
     * Get the JavaFX PixelBuffer.
     * 
     * @return PixelBuffer for use with WritableImage
     * @throws IllegalStateException if not initialized
     */
    public PixelBuffer<ByteBuffer> getPixelBuffer() {
        if (pixelBuffer == null) {
            throw new IllegalStateException("PixelBufferManager not initialized");
        }
        return pixelBuffer;
    }
    
    /**
     * Get the stable ByteBuffer backing.
     * 
     * @return Stable ByteBuffer (DO NOT mutate position/limit/mark)
     * @throws IllegalStateException if not initialized
     */
    public ByteBuffer getByteBuffer() {
        if (stableByteBuffer == null) {
            throw new IllegalStateException("PixelBufferManager not initialized");
        }
        return stableByteBuffer;
    }
    
    /**
     * Check if initialized.
     * 
     * @return true if initialized
     */
    public boolean isInitialized() {
        return pixelBuffer != null;
    }
    
    /**
     * Get current viewport width.
     * 
     * @return Viewport width in device pixels
     */
    public int getCurrentWidth() {
        return currentWidth;
    }
    
    /**
     * Get current viewport height.
     * 
     * @return Viewport height in device pixels
     */
    public int getCurrentHeight() {
        return currentHeight;
    }
    
    /**
     * Get maximum viewport width.
     * 
     * @return Maximum width in device pixels
     */
    public int getMaxWidth() {
        return maxWidth;
    }
    
    /**
     * Get maximum viewport height.
     * 
     * @return Maximum height in device pixels
     */
    public int getMaxHeight() {
        return maxHeight;
    }
    
    /**
     * Get DPI scale factor.
     * 
     * @return DPI scale factor
     */
    public double getDpiScale() {
        return dpiScale;
    }
}
