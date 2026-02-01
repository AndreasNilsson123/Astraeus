package com.astraeus.rendering;

/**
 * Viewport manager for integrating native framebuffer with JavaFX.
 * This will handle zero-copy readback and display.
 */
public class ViewportManager {
    
    private int width;
    private int height;
    
    public ViewportManager(int width, int height) {
        this.width = width;
        this.height = height;
    }
    
    /**
     * Update viewport size.
     */
    public void resize(int width, int height) {
        this.width = width;
        this.height = height;
    }
    
    /**
     * Get framebuffer data for display.
     * TODO: Implement zero-copy readback using MemorySegment
     */
    public byte[] getFramebufferData() {
        // Placeholder
        return new byte[width * height * 4];
    }
    
    public int getWidth() {
        return width;
    }
    
    public int getHeight() {
        return height;
    }
}
