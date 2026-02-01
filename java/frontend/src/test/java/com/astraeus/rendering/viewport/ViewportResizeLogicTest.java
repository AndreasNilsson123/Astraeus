package com.astraeus.rendering.viewport;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Test suite for viewport resize logic and dimension calculations.
 * 
 * <p>Verifies:
 * - Logical size to device pixel conversion
 * - HiDPI scaling calculations
 * - Aspect ratio preservation
 * - Viewport dimension constraints
 */
@DisplayName("Viewport Resize Logic Tests")
class ViewportResizeLogicTest {
    
    @ParameterizedTest(name = "logical={0}x{1}, scale={2} -> device={3}x{4}")
    @CsvSource({
        "1280, 720, 1.0, 1280, 720",
        "1280, 720, 2.0, 2560, 1440",
        "1920, 1080, 1.5, 2880, 1620",
        "800, 600, 1.25, 1000, 750",
        "640, 480, 1.0, 640, 480"
    })
    @DisplayName("Logical to device pixel conversion should be correct")
    void logicalToDevicePixels_shouldBeCorrect(
            int logicalWidth, int logicalHeight, double scale,
            int expectedDeviceWidth, int expectedDeviceHeight) {
        
        int deviceWidth = (int) Math.ceil(logicalWidth * scale);
        int deviceHeight = (int) Math.ceil(logicalHeight * scale);
        
        assertThat(deviceWidth).isEqualTo(expectedDeviceWidth);
        assertThat(deviceHeight).isEqualTo(expectedDeviceHeight);
    }
    
    @ParameterizedTest(name = "device={0}x{1} -> aspect={2}")
    @CsvSource({
        "1280, 720, 1.7777778",
        "1920, 1080, 1.7777778",
        "2560, 1440, 1.7777778",
        "1024, 768, 1.3333333",
        "640, 480, 1.3333333",
        "3840, 2160, 1.7777778"
    })
    @DisplayName("Aspect ratio calculation should be correct")
    void aspectRatio_shouldBeCorrect(int width, int height, float expectedAspect) {
        float aspect = (float) width / height;
        
        assertThat(aspect)
            .as("Aspect ratio for %dx%d", width, height)
            .isCloseTo(expectedAspect, org.assertj.core.data.Offset.offset(0.0001f));
    }
    
    @Test
    @DisplayName("Viewport dimensions should be positive")
    void viewportDimensions_shouldBePositive() {
        // Test valid dimensions
        assertThat(isValidViewportSize(1280, 720)).isTrue();
        assertThat(isValidViewportSize(1, 1)).isTrue();
        assertThat(isValidViewportSize(8192, 8192)).isTrue();
        
        // Test invalid dimensions
        assertThat(isValidViewportSize(0, 720)).isFalse();
        assertThat(isValidViewportSize(1280, 0)).isFalse();
        assertThat(isValidViewportSize(-100, 720)).isFalse();
        assertThat(isValidViewportSize(1280, -100)).isFalse();
    }
    
    @Test
    @DisplayName("Viewport should handle maximum dimensions")
    void viewport_shouldHandleMaximumDimensions() {
        int maxWidth = 3840;
        int maxHeight = 2160;
        
        // Within bounds
        assertThat(clampToMax(1920, 1080, maxWidth, maxHeight))
            .containsExactly(1920, 1080);
        
        // Exceeds width
        assertThat(clampToMax(4096, 2160, maxWidth, maxHeight))
            .containsExactly(3840, 2160);
        
        // Exceeds height
        assertThat(clampToMax(3840, 2400, maxWidth, maxHeight))
            .containsExactly(3840, 2160);
        
        // Exceeds both
        assertThat(clampToMax(5120, 2880, maxWidth, maxHeight))
            .containsExactly(3840, 2160);
    }
    
    @Test
    @DisplayName("Viewport resize should maintain aspect ratio when clamping")
    void viewportResize_shouldMaintainAspectRatioWhenClamping() {
        int maxWidth = 2560;
        int maxHeight = 1440;
        
        // Original 16:9 ratio
        int[] result = clampToMaxMaintainAspect(3840, 2160, maxWidth, maxHeight);
        
        assertThat(result[0]).isLessThanOrEqualTo(maxWidth);
        assertThat(result[1]).isLessThanOrEqualTo(maxHeight);
        
        float originalAspect = 3840f / 2160f;
        float clampedAspect = (float) result[0] / result[1];
        
        assertThat(clampedAspect)
            .as("Clamped aspect ratio should match original")
            .isCloseTo(originalAspect, org.assertj.core.data.Offset.offset(0.01f));
    }
    
    @Test
    @DisplayName("HiDPI scale factor should be positive")
    void hiDpiScale_shouldBePositive() {
        assertThat(isValidScale(1.0)).isTrue();
        assertThat(isValidScale(1.5)).isTrue();
        assertThat(isValidScale(2.0)).isTrue();
        
        assertThat(isValidScale(0.0)).isFalse();
        assertThat(isValidScale(-1.0)).isFalse();
    }
    
    @Test
    @DisplayName("Viewport update region should be within bounds")
    void viewportUpdateRegion_shouldBeWithinBounds() {
        int viewportWidth = 1920;
        int viewportHeight = 1080;
        
        // Full update
        assertThat(isValidUpdateRegion(0, 0, 1920, 1080, viewportWidth, viewportHeight))
            .isTrue();
        
        // Partial update
        assertThat(isValidUpdateRegion(100, 100, 800, 600, viewportWidth, viewportHeight))
            .isTrue();
        
        // Out of bounds - exceeds width
        assertThat(isValidUpdateRegion(0, 0, 2000, 1080, viewportWidth, viewportHeight))
            .isFalse();
        
        // Out of bounds - exceeds height
        assertThat(isValidUpdateRegion(0, 0, 1920, 1200, viewportWidth, viewportHeight))
            .isFalse();
        
        // Negative offset
        assertThat(isValidUpdateRegion(-10, 0, 1920, 1080, viewportWidth, viewportHeight))
            .isFalse();
    }
    
    // Helper methods
    
    private boolean isValidViewportSize(int width, int height) {
        return width > 0 && height > 0;
    }
    
    private int[] clampToMax(int width, int height, int maxWidth, int maxHeight) {
        return new int[] {
            Math.min(width, maxWidth),
            Math.min(height, maxHeight)
        };
    }
    
    private int[] clampToMaxMaintainAspect(int width, int height, int maxWidth, int maxHeight) {
        float aspect = (float) width / height;
        
        if (width > maxWidth) {
            width = maxWidth;
            height = (int) (width / aspect);
        }
        
        if (height > maxHeight) {
            height = maxHeight;
            width = (int) (height * aspect);
        }
        
        return new int[] { width, height };
    }
    
    private boolean isValidScale(double scale) {
        return scale > 0.0;
    }
    
    private boolean isValidUpdateRegion(int x, int y, int width, int height, 
                                       int viewportWidth, int viewportHeight) {
        return x >= 0 && y >= 0 && 
               x + width <= viewportWidth && 
               y + height <= viewportHeight;
    }
}
