package com.astraeus.tools;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.nio.ByteBuffer;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Test suite for TestPatternGenerator.
 * 
 * <p>Verifies:
 * - Test pattern generation correctness
 * - Buffer format compliance (BGRA8)
 * - Stride handling
 * - Pixel color values
 */
@DisplayName("Test Pattern Generator Tests")
class TestPatternGeneratorTest {
    
    @Test
    @DisplayName("Gradient pattern should fill buffer completely")
    void gradientPattern_shouldFillBufferCompletely() {
        int width = 100;
        int height = 100;
        int stride = width * 4;
        ByteBuffer buffer = ByteBuffer.allocateDirect(height * stride);
        
        TestPatternGenerator.fillGradient(buffer, width, height, stride);
        
        // Verify buffer is filled (no zero bytes at known positions)
        buffer.position(0);
        
        // Check first pixel (should be near black with some blue)
        byte b0 = buffer.get(0);
        byte g0 = buffer.get(1);
        byte r0 = buffer.get(2);
        byte a0 = buffer.get(3);
        
        assertThat(a0 & 0xFF).isEqualTo(255); // Alpha should be 255
        assertThat(b0 & 0xFF).isEqualTo(128); // Blue fixed at 128
        
        // Check last pixel (should be near white/green)
        int lastPixelOffset = (height - 1) * stride + (width - 1) * 4;
        byte bLast = buffer.get(lastPixelOffset + 0);
        byte gLast = buffer.get(lastPixelOffset + 1);
        byte rLast = buffer.get(lastPixelOffset + 2);
        byte aLast = buffer.get(lastPixelOffset + 3);
        
        assertThat(aLast & 0xFF).isEqualTo(255);
        assertThat(rLast & 0xFF).isCloseTo(255, org.assertj.core.data.Offset.offset(5));
        assertThat(gLast & 0xFF).isCloseTo(255, org.assertj.core.data.Offset.offset(5));
    }
    
    @Test
    @DisplayName("Checkerboard pattern should alternate colors")
    void checkerboardPattern_shouldAlternateColors() {
        int width = 64;
        int height = 64;
        int stride = width * 4;
        int squareSize = 8;
        ByteBuffer buffer = ByteBuffer.allocateDirect(height * stride);
        
        TestPatternGenerator.fillCheckerboard(buffer, width, height, stride, squareSize);
        
        buffer.position(0);
        
        // Check first square (should be white or black)
        byte b0 = buffer.get(0);
        byte g0 = buffer.get(1);
        byte r0 = buffer.get(2);
        
        boolean isWhiteOrBlack = 
            ((b0 & 0xFF) == 255 && (g0 & 0xFF) == 255 && (r0 & 0xFF) == 255) ||
            ((b0 & 0xFF) == 0 && (g0 & 0xFF) == 0 && (r0 & 0xFF) == 0);
        
        assertThat(isWhiteOrBlack).isTrue();
        
        // Check pixel in adjacent square (should be opposite color)
        int adjPixelOffset = squareSize * 4; // One square to the right
        byte bAdj = buffer.get(adjPixelOffset + 0);
        byte gAdj = buffer.get(adjPixelOffset + 1);
        byte rAdj = buffer.get(adjPixelOffset + 2);
        
        // Adjacent square should be opposite color
        boolean isOpposite = 
            ((b0 & 0xFF) == 255) != ((bAdj & 0xFF) == 255);
        
        assertThat(isOpposite).isTrue();
    }
    
    @Test
    @DisplayName("Grid pattern should have distinct grid lines")
    void gridPattern_shouldHaveDistinctGridLines() {
        int width = 100;
        int height = 100;
        int stride = width * 4;
        int gridSpacing = 10;
        ByteBuffer buffer = ByteBuffer.allocateDirect(height * stride);
        
        TestPatternGenerator.fillGrid(buffer, width, height, stride, gridSpacing);
        
        buffer.position(0);
        
        // Check a grid line pixel (x=0, y=0 should be a vertical red line)
        byte b = buffer.get(0);
        byte g = buffer.get(1);
        byte r = buffer.get(2);
        
        // Should be red (vertical line) or green (horizontal line)
        boolean isGridLine = 
            ((r & 0xFF) == 255 && (g & 0xFF) == 0) ||  // Red vertical
            ((g & 0xFF) == 255 && (r & 0xFF) == 0);    // Green horizontal
        
        assertThat(isGridLine).isTrue();
    }
    
    @Test
    @DisplayName("Color bands pattern should have distinct bands")
    void colorBandsPattern_shouldHaveDistinctBands() {
        int width = 100;
        int height = 160; // 8 bands * 20 pixels each
        int stride = width * 4;
        int bandHeight = 20;
        ByteBuffer buffer = ByteBuffer.allocateDirect(height * stride);
        
        TestPatternGenerator.fillColorBands(buffer, width, height, stride, bandHeight);
        
        buffer.position(0);
        
        // Check first band (should be blue)
        byte b0 = buffer.get(0);
        byte g0 = buffer.get(1);
        byte r0 = buffer.get(2);
        
        assertThat(b0 & 0xFF).isEqualTo(255);
        assertThat(g0 & 0xFF).isEqualTo(0);
        assertThat(r0 & 0xFF).isEqualTo(0);
        
        // Check second band (should be green)
        int secondBandOffset = bandHeight * stride;
        byte b1 = buffer.get(secondBandOffset + 0);
        byte g1 = buffer.get(secondBandOffset + 1);
        byte r1 = buffer.get(secondBandOffset + 2);
        
        assertThat(b1 & 0xFF).isEqualTo(0);
        assertThat(g1 & 0xFF).isEqualTo(255);
        assertThat(r1 & 0xFF).isEqualTo(0);
    }
    
    @Test
    @DisplayName("Quadrants pattern should have four distinct colors")
    void quadrantsPattern_shouldHaveFourDistinctColors() {
        int width = 100;
        int height = 100;
        int stride = width * 4;
        ByteBuffer buffer = ByteBuffer.allocateDirect(height * stride);
        
        TestPatternGenerator.fillQuadrants(buffer, width, height, stride);
        
        buffer.position(0);
        
        // Top-left (should be red)
        int tlOffset = 10 * stride + 10 * 4;
        byte bTL = buffer.get(tlOffset + 0);
        byte gTL = buffer.get(tlOffset + 1);
        byte rTL = buffer.get(tlOffset + 2);
        
        assertThat(rTL & 0xFF).isEqualTo(255);
        assertThat(gTL & 0xFF).isEqualTo(0);
        assertThat(bTL & 0xFF).isEqualTo(0);
        
        // Top-right (should be green)
        int trOffset = 10 * stride + 60 * 4;
        byte bTR = buffer.get(trOffset + 0);
        byte gTR = buffer.get(trOffset + 1);
        byte rTR = buffer.get(trOffset + 2);
        
        assertThat(rTR & 0xFF).isEqualTo(0);
        assertThat(gTR & 0xFF).isEqualTo(255);
        assertThat(bTR & 0xFF).isEqualTo(0);
        
        // Bottom-left (should be blue)
        int blOffset = 60 * stride + 10 * 4;
        byte bBL = buffer.get(blOffset + 0);
        byte gBL = buffer.get(blOffset + 1);
        byte rBL = buffer.get(blOffset + 2);
        
        assertThat(rBL & 0xFF).isEqualTo(0);
        assertThat(gBL & 0xFF).isEqualTo(0);
        assertThat(bBL & 0xFF).isEqualTo(255);
        
        // Bottom-right (should be yellow)
        int brOffset = 60 * stride + 60 * 4;
        byte bBR = buffer.get(brOffset + 0);
        byte gBR = buffer.get(brOffset + 1);
        byte rBR = buffer.get(brOffset + 2);
        
        assertThat(rBR & 0xFF).isEqualTo(255);
        assertThat(gBR & 0xFF).isEqualTo(255);
        assertThat(bBR & 0xFF).isEqualTo(0);
    }
    
    @Test
    @DisplayName("Pattern generation should handle non-square dimensions")
    void patternGeneration_shouldHandleNonSquareDimensions() {
        int width = 160;
        int height = 90;
        int stride = width * 4;
        ByteBuffer buffer = ByteBuffer.allocateDirect(height * stride);
        
        // Should not throw for any pattern
        TestPatternGenerator.fillGradient(buffer, width, height, stride);
        TestPatternGenerator.fillCheckerboard(buffer, width, height, stride, 10);
        TestPatternGenerator.fillGrid(buffer, width, height, stride, 10);
        TestPatternGenerator.fillColorBands(buffer, width, height, stride, 10);
        TestPatternGenerator.fillQuadrants(buffer, width, height, stride);
    }
    
    @Test
    @DisplayName("Pattern generation should handle stride with padding")
    void patternGeneration_shouldHandleStrideWithPadding() {
        int width = 100;
        int height = 100;
        int stride = width * 4 + 64; // Add 64 bytes padding per row
        ByteBuffer buffer = ByteBuffer.allocateDirect(height * stride);
        
        // Should not throw and should respect stride
        TestPatternGenerator.fillGradient(buffer, width, height, stride);
        
        // Verify that pixels are written at correct stride-aligned positions
        // First pixel of second row should be at stride offset, not width*4
        int secondRowOffset = stride;
        buffer.position(secondRowOffset);
        
        byte alpha = buffer.get(secondRowOffset + 3);
        assertThat(alpha & 0xFF).isEqualTo(255);
    }
}
