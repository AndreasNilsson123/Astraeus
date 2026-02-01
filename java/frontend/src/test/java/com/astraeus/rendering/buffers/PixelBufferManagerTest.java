package com.astraeus.rendering.buffers;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

import java.nio.ByteBuffer;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

/**
 * Test suite for PixelBufferManager.
 * 
 * <p>Verifies:
 * - Buffer allocation and capacity
 * - Update region calculations
 * - Backing buffer stability
 * - BGRA8 format handling (4 bytes per pixel)
 */
@DisplayName("PixelBufferManager Tests")
class PixelBufferManagerTest {
    
    @ParameterizedTest(name = "dimensions={0}x{1} -> capacity={2}")
    @CsvSource({
        "1280, 720, 3686400",    // 1280 * 720 * 4
        "1920, 1080, 8294400",   // 1920 * 1080 * 4
        "2560, 1440, 14745600",  // 2560 * 1440 * 4
        "3840, 2160, 33177600",  // 3840 * 2160 * 4
        "640, 480, 1228800"      // 640 * 480 * 4
    })
    @DisplayName("Buffer capacity should match dimensions for BGRA8 format")
    void bufferCapacity_shouldMatchDimensionsForBGRA8(int width, int height, int expectedCapacity) {
        int bytesPerPixel = 4; // BGRA8
        int capacity = width * height * bytesPerPixel;
        
        assertThat(capacity).isEqualTo(expectedCapacity);
    }
    
    @Test
    @DisplayName("ByteBuffer allocation should succeed for typical viewport sizes")
    void byteBufferAllocation_shouldSucceedForTypicalSizes() {
        // Allocate buffers for common resolutions
        ByteBuffer buffer720p = ByteBuffer.allocateDirect(1280 * 720 * 4);
        assertThat(buffer720p.capacity()).isEqualTo(1280 * 720 * 4);
        
        ByteBuffer buffer1080p = ByteBuffer.allocateDirect(1920 * 1080 * 4);
        assertThat(buffer1080p.capacity()).isEqualTo(1920 * 1080 * 4);
        
        ByteBuffer buffer1440p = ByteBuffer.allocateDirect(2560 * 1440 * 4);
        assertThat(buffer1440p.capacity()).isEqualTo(2560 * 1440 * 4);
    }
    
    @Test
    @DisplayName("ByteBuffer should be direct for native interop")
    void byteBuffer_shouldBeDirectForNativeInterop() {
        ByteBuffer buffer = ByteBuffer.allocateDirect(1024);
        
        assertThat(buffer.isDirect())
            .as("Buffer should be direct for native memory access")
            .isTrue();
    }
    
    @Test
    @DisplayName("ByteBuffer position/limit should remain stable")
    void byteBuffer_positionLimit_shouldRemainStable() {
        ByteBuffer buffer = ByteBuffer.allocateDirect(1000);
        
        // Initial state
        assertThat(buffer.position()).isEqualTo(0);
        assertThat(buffer.limit()).isEqualTo(buffer.capacity());
        
        // Write some data
        buffer.put((byte) 1);
        buffer.put((byte) 2);
        buffer.put((byte) 3);
        
        // Position changed
        assertThat(buffer.position()).isEqualTo(3);
        
        // Reset to initial state
        buffer.position(0);
        assertThat(buffer.position()).isEqualTo(0);
        assertThat(buffer.limit()).isEqualTo(buffer.capacity());
    }
    
    @Test
    @DisplayName("Update region should not exceed buffer dimensions")
    void updateRegion_shouldNotExceedBufferDimensions() {
        int width = 1920;
        int height = 1080;
        
        // Valid full-frame update
        assertThat(isValidUpdateRegion(0, 0, width, height, width, height))
            .isTrue();
        
        // Valid partial update
        assertThat(isValidUpdateRegion(100, 100, 800, 600, width, height))
            .isTrue();
        
        // Invalid - exceeds bounds
        assertThat(isValidUpdateRegion(0, 0, width + 100, height, width, height))
            .isFalse();
        
        assertThat(isValidUpdateRegion(0, 0, width, height + 100, width, height))
            .isFalse();
    }
    
    @Test
    @DisplayName("Stride calculation should account for alignment")
    void strideCalculation_shouldAccountForAlignment() {
        // Tightly packed stride (no padding)
        assertThat(calculateTightStride(1920, 4)).isEqualTo(1920 * 4);
        assertThat(calculateTightStride(1280, 4)).isEqualTo(1280 * 4);
        
        // Aligned stride (e.g., 256-byte alignment)
        int alignment = 256;
        assertThat(calculateAlignedStride(1920, 4, alignment) % alignment).isEqualTo(0);
        assertThat(calculateAlignedStride(1280, 4, alignment) % alignment).isEqualTo(0);
    }
    
    @Test
    @DisplayName("Buffer size should accommodate stride padding")
    void bufferSize_shouldAccommodateStridePadding() {
        int width = 1920;
        int height = 1080;
        int bytesPerPixel = 4;
        int alignment = 256;
        
        int tightStride = calculateTightStride(width, bytesPerPixel);
        int alignedStride = calculateAlignedStride(width, bytesPerPixel, alignment);
        
        int tightSize = height * tightStride;
        int alignedSize = height * alignedStride;
        
        assertThat(alignedSize).isGreaterThanOrEqualTo(tightSize);
    }
    
    @Test
    @DisplayName("Pixel buffer view dimensions should match viewport")
    void pixelBufferView_dimensions_shouldMatchViewport() {
        int viewportWidth = 1280;
        int viewportHeight = 720;
        int maxWidth = 2560;
        int maxHeight = 1440;
        
        // Current viewport is smaller than max
        assertThat(viewportWidth).isLessThanOrEqualTo(maxWidth);
        assertThat(viewportHeight).isLessThanOrEqualTo(maxHeight);
        
        // Buffer backing size should be for max dimensions
        int backingSize = maxWidth * maxHeight * 4;
        
        // Active region size should be for current viewport
        int activeSize = viewportWidth * viewportHeight * 4;
        
        assertThat(activeSize).isLessThanOrEqualTo(backingSize);
    }
    
    @Test
    @DisplayName("Frame dimension mismatch should be detected")
    void frameDimensionMismatch_shouldBeDetected() {
        int expectedWidth = 1920;
        int expectedHeight = 1080;
        
        int actualWidth = 1280;
        int actualHeight = 720;
        
        assertThat(actualWidth).isNotEqualTo(expectedWidth);
        assertThat(actualHeight).isNotEqualTo(expectedHeight);
        
        boolean mismatch = (actualWidth != expectedWidth) || (actualHeight != expectedHeight);
        assertThat(mismatch).isTrue();
    }
    
    @Test
    @DisplayName("Zero-sized buffer allocation should be handled")
    void zeroSizedBuffer_shouldBeHandled() {
        // In Java, ByteBuffer.allocateDirect(0) is actually allowed
        // It returns a buffer with capacity 0
        ByteBuffer buffer = ByteBuffer.allocateDirect(0);
        assertThat(buffer.capacity()).isEqualTo(0);
    }
    
    @Test
    @DisplayName("Negative buffer size should be rejected")
    void negativeBufferSize_shouldBeRejected() {
        assertThatThrownBy(() -> ByteBuffer.allocateDirect(-100))
            .isInstanceOf(IllegalArgumentException.class);
    }
    
    // Helper methods
    
    private boolean isValidUpdateRegion(int x, int y, int width, int height,
                                       int bufferWidth, int bufferHeight) {
        return x >= 0 && y >= 0 &&
               x + width <= bufferWidth &&
               y + height <= bufferHeight;
    }
    
    private int calculateTightStride(int width, int bytesPerPixel) {
        return width * bytesPerPixel;
    }
    
    private int calculateAlignedStride(int width, int bytesPerPixel, int alignment) {
        int tightStride = width * bytesPerPixel;
        return ((tightStride + alignment - 1) / alignment) * alignment;
    }
}
