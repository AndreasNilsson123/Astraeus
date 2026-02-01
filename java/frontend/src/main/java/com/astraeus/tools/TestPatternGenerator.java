package com.astraeus.tools;

import java.nio.ByteBuffer;

/**
 * Utility for generating test patterns in pixel buffers.
 * Useful for diagnosing rendering and buffer layout issues.
 * 
 * <p>Test patterns include:
 * <ul>
 *   <li>Gradient: Smooth color gradients to verify continuity</li>
 *   <li>Checkerboard: Alternating colors to verify alignment</li>
 *   <li>Grid: Grid lines to verify stride and row alignment</li>
 *   <li>Color bands: Horizontal/vertical color bands to verify row/column order</li>
 * </ul>
 * 
 * <p><b>Usage:</b></p>
 * <pre>
 * // Generate a gradient pattern
 * ByteBuffer buffer = ...;
 * TestPatternGenerator.fillGradient(buffer, width, height, stride);
 * 
 * // Generate a checkerboard pattern
 * TestPatternGenerator.fillCheckerboard(buffer, width, height, stride, 32);
 * </pre>
 */
public class TestPatternGenerator {
    
    /**
     * Fill buffer with a horizontal gradient (red -> green).
     * Useful for verifying horizontal continuity and stride handling.
     * 
     * @param buffer Target ByteBuffer (BGRA8 format)
     * @param width Width in pixels
     * @param height Height in pixels
     * @param stride Row stride in bytes
     */
    public static void fillGradient(ByteBuffer buffer, int width, int height, int stride) {
        buffer.clear();
        
        for (int y = 0; y < height; y++) {
            int rowOffset = y * stride;
            
            for (int x = 0; x < width; x++) {
                int pixelOffset = rowOffset + x * 4;
                
                // Calculate gradient colors
                int red = (int) ((x / (float) width) * 255);
                int green = (int) ((y / (float) height) * 255);
                int blue = 128;
                int alpha = 255;
                
                // BGRA format
                buffer.put(pixelOffset + 0, (byte) blue);
                buffer.put(pixelOffset + 1, (byte) green);
                buffer.put(pixelOffset + 2, (byte) red);
                buffer.put(pixelOffset + 3, (byte) alpha);
            }
        }
        
        buffer.clear();
    }
    
    /**
     * Fill buffer with a checkerboard pattern.
     * Useful for verifying alignment and stride handling.
     * 
     * @param buffer Target ByteBuffer (BGRA8 format)
     * @param width Width in pixels
     * @param height Height in pixels
     * @param stride Row stride in bytes
     * @param squareSize Size of each checker square in pixels
     */
    public static void fillCheckerboard(ByteBuffer buffer, int width, int height, int stride, int squareSize) {
        buffer.clear();
        
        for (int y = 0; y < height; y++) {
            int rowOffset = y * stride;
            int checkY = (y / squareSize) % 2;
            
            for (int x = 0; x < width; x++) {
                int pixelOffset = rowOffset + x * 4;
                int checkX = (x / squareSize) % 2;
                
                // Alternate between white and black
                boolean isWhite = (checkX + checkY) % 2 == 0;
                byte value = isWhite ? (byte) 255 : (byte) 0;
                
                // BGRA format
                buffer.put(pixelOffset + 0, value); // B
                buffer.put(pixelOffset + 1, value); // G
                buffer.put(pixelOffset + 2, value); // R
                buffer.put(pixelOffset + 3, (byte) 255); // A
            }
        }
        
        buffer.clear();
    }
    
    /**
     * Fill buffer with a grid pattern with colored lines.
     * Useful for verifying row/column alignment.
     * 
     * @param buffer Target ByteBuffer (BGRA8 format)
     * @param width Width in pixels
     * @param height Height in pixels
     * @param stride Row stride in bytes
     * @param gridSpacing Spacing between grid lines in pixels
     */
    public static void fillGrid(ByteBuffer buffer, int width, int height, int stride, int gridSpacing) {
        buffer.clear();
        
        // Fill background (dark gray)
        for (int y = 0; y < height; y++) {
            int rowOffset = y * stride;
            
            for (int x = 0; x < width; x++) {
                int pixelOffset = rowOffset + x * 4;
                
                buffer.put(pixelOffset + 0, (byte) 64);  // B
                buffer.put(pixelOffset + 1, (byte) 64);  // G
                buffer.put(pixelOffset + 2, (byte) 64);  // R
                buffer.put(pixelOffset + 3, (byte) 255); // A
            }
        }
        
        // Draw vertical lines (red)
        for (int x = 0; x < width; x += gridSpacing) {
            for (int y = 0; y < height; y++) {
                int pixelOffset = y * stride + x * 4;
                
                buffer.put(pixelOffset + 0, (byte) 0);   // B
                buffer.put(pixelOffset + 1, (byte) 0);   // G
                buffer.put(pixelOffset + 2, (byte) 255); // R (red)
                buffer.put(pixelOffset + 3, (byte) 255); // A
            }
        }
        
        // Draw horizontal lines (green)
        for (int y = 0; y < height; y += gridSpacing) {
            int rowOffset = y * stride;
            
            for (int x = 0; x < width; x++) {
                int pixelOffset = rowOffset + x * 4;
                
                buffer.put(pixelOffset + 0, (byte) 0);   // B
                buffer.put(pixelOffset + 1, (byte) 255); // G (green)
                buffer.put(pixelOffset + 2, (byte) 0);   // R
                buffer.put(pixelOffset + 3, (byte) 255); // A
            }
        }
        
        buffer.clear();
    }
    
    /**
     * Fill buffer with horizontal color bands.
     * Useful for verifying row order and stride issues.
     * Each band is a different color.
     * 
     * @param buffer Target ByteBuffer (BGRA8 format)
     * @param width Width in pixels
     * @param height Height in pixels
     * @param stride Row stride in bytes
     * @param bandHeight Height of each color band in pixels
     */
    public static void fillColorBands(ByteBuffer buffer, int width, int height, int stride, int bandHeight) {
        buffer.clear();
        
        // Define colors for bands (BGRA format)
        byte[][] colors = {
            {(byte) 255, (byte) 0,   (byte) 0,   (byte) 255}, // Blue
            {(byte) 0,   (byte) 255, (byte) 0,   (byte) 255}, // Green
            {(byte) 0,   (byte) 0,   (byte) 255, (byte) 255}, // Red
            {(byte) 255, (byte) 255, (byte) 0,   (byte) 255}, // Cyan
            {(byte) 255, (byte) 0,   (byte) 255, (byte) 255}, // Magenta
            {(byte) 0,   (byte) 255, (byte) 255, (byte) 255}, // Yellow
            {(byte) 255, (byte) 255, (byte) 255, (byte) 255}, // White
            {(byte) 128, (byte) 128, (byte) 128, (byte) 255}  // Gray
        };
        
        for (int y = 0; y < height; y++) {
            int rowOffset = y * stride;
            int colorIndex = (y / bandHeight) % colors.length;
            byte[] color = colors[colorIndex];
            
            for (int x = 0; x < width; x++) {
                int pixelOffset = rowOffset + x * 4;
                
                buffer.put(pixelOffset + 0, color[0]); // B
                buffer.put(pixelOffset + 1, color[1]); // G
                buffer.put(pixelOffset + 2, color[2]); // R
                buffer.put(pixelOffset + 3, color[3]); // A
            }
        }
        
        buffer.clear();
    }
    
    /**
     * Fill buffer with a diagnostic pattern that shows all quadrants.
     * Top-left: Red, Top-right: Green, Bottom-left: Blue, Bottom-right: Yellow
     * Useful for verifying correct frame boundaries and orientation.
     * 
     * @param buffer Target ByteBuffer (BGRA8 format)
     * @param width Width in pixels
     * @param height Height in pixels
     * @param stride Row stride in bytes
     */
    public static void fillQuadrants(ByteBuffer buffer, int width, int height, int stride) {
        buffer.clear();
        
        int midX = width / 2;
        int midY = height / 2;
        
        for (int y = 0; y < height; y++) {
            int rowOffset = y * stride;
            
            for (int x = 0; x < width; x++) {
                int pixelOffset = rowOffset + x * 4;
                
                byte b, g, r;
                if (y < midY) {
                    if (x < midX) {
                        // Top-left: Red
                        r = (byte) 255;
                        g = 0;
                        b = 0;
                    } else {
                        // Top-right: Green
                        r = 0;
                        g = (byte) 255;
                        b = 0;
                    }
                } else {
                    if (x < midX) {
                        // Bottom-left: Blue
                        r = 0;
                        g = 0;
                        b = (byte) 255;
                    } else {
                        // Bottom-right: Yellow
                        r = (byte) 255;
                        g = (byte) 255;
                        b = 0;
                    }
                }
                
                buffer.put(pixelOffset + 0, b); // B
                buffer.put(pixelOffset + 1, g); // G
                buffer.put(pixelOffset + 2, r); // R
                buffer.put(pixelOffset + 3, (byte) 255); // A
            }
        }
        
        buffer.clear();
    }
}
