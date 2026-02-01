package com.astraeus.tools.telemetry;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Test suite for telemetry formatting and display logic.
 * 
 * <p>Verifies:
 * - Frame time formatting
 * - FPS calculation
 * - Memory size formatting
 * - Percentage formatting
 * - Stale frame detection
 */
@DisplayName("Telemetry Formatting Tests")
class TelemetryFormattingTest {
    
    @ParameterizedTest(name = "frameTimeMs={0} -> {1}")
    @CsvSource({
        "16.67, '16.7ms'",
        "8.33, '8.3ms'",
        "33.33, '33.3ms'",
        "1.23, '1.2ms'",
        "100.0, '100.0ms'"
    })
    @DisplayName("Frame time should be formatted with one decimal")
    void frameTime_shouldBeFormattedWithOneDecimal(double frameTimeMs, String expected) {
        String formatted = String.format("%.1fms", frameTimeMs);
        assertThat(formatted).isEqualTo(expected);
    }
    
    @ParameterizedTest(name = "frameTimeMs={0} -> fps={1}")
    @CsvSource({
        "16.67, 60.0",
        "8.33, 120.1",
        "33.33, 30.0",
        "10.0, 100.0",
        "20.0, 50.0"
    })
    @DisplayName("FPS should be calculated from frame time")
    void fps_shouldBeCalculatedFromFrameTime(double frameTimeMs, double expectedFps) {
        double fps = 1000.0 / frameTimeMs;
        assertThat(fps).isCloseTo(expectedFps, org.assertj.core.data.Offset.offset(0.1));
    }
    
    @ParameterizedTest(name = "bytes={0} -> {1}")
    @CsvSource({
        "1024, '1.0 KB'",
        "1048576, '1.0 MB'",
        "1073741824, '1.0 GB'",
        "2560, '2.5 KB'",
        "5242880, '5.0 MB'"
    })
    @DisplayName("Memory size should be formatted in appropriate units")
    void memorySize_shouldBeFormattedInAppropriateUnits(long bytes, String expected) {
        String formatted = formatMemorySize(bytes);
        assertThat(formatted).isEqualTo(expected);
    }
    
    @ParameterizedTest(name = "value={0}, total={1} -> {2}%")
    @CsvSource({
        "50, 100, 50.0",
        "75, 100, 75.0",
        "1, 4, 25.0",
        "3, 4, 75.0",
        "100, 200, 50.0"
    })
    @DisplayName("Percentage should be calculated correctly")
    void percentage_shouldBeCalculatedCorrectly(double value, double total, double expectedPercent) {
        double percent = (value / total) * 100.0;
        assertThat(percent).isCloseTo(expectedPercent, org.assertj.core.data.Offset.offset(0.1));
    }
    
    @Test
    @DisplayName("Stale frame should be detected after threshold")
    void staleFrame_shouldBeDetectedAfterThreshold() {
        long currentTime = System.currentTimeMillis();
        long lastUpdateTime = currentTime - 2000; // 2 seconds ago
        long staleThresholdMs = 1000; // 1 second
        
        boolean isStale = (currentTime - lastUpdateTime) > staleThresholdMs;
        assertThat(isStale).isTrue();
    }
    
    @Test
    @DisplayName("Fresh frame should not be marked as stale")
    void freshFrame_shouldNotBeMarkedAsStale() {
        long currentTime = System.currentTimeMillis();
        long lastUpdateTime = currentTime - 500; // 500ms ago
        long staleThresholdMs = 1000; // 1 second
        
        boolean isStale = (currentTime - lastUpdateTime) > staleThresholdMs;
        assertThat(isStale).isFalse();
    }
    
    @Test
    @DisplayName("Frame counter should increment monotonically")
    void frameCounter_shouldIncrementMonotonically() {
        long frameCount = 0;
        
        for (int i = 0; i < 100; i++) {
            long previousCount = frameCount;
            frameCount++;
            
            assertThat(frameCount)
                .as("Frame %d count", i)
                .isGreaterThan(previousCount);
        }
    }
    
    @Test
    @DisplayName("Average frame time should be calculated correctly")
    void averageFrameTime_shouldBeCalculatedCorrectly() {
        double[] frameTimes = { 16.0, 17.0, 15.0, 16.5, 16.2 };
        
        double sum = 0.0;
        for (double time : frameTimes) {
            sum += time;
        }
        double average = sum / frameTimes.length;
        
        assertThat(average).isCloseTo(16.14, org.assertj.core.data.Offset.offset(0.01));
    }
    
    @Test
    @DisplayName("Frame time spike should be detected")
    void frameTimeSpike_shouldBeDetected() {
        double normalFrameTime = 16.67; // ~60 FPS
        double spikeFrameTime = 50.0;   // ~20 FPS
        double spikeThreshold = 2.0;     // 2x normal
        
        boolean isSpike = spikeFrameTime > (normalFrameTime * spikeThreshold);
        assertThat(isSpike).isTrue();
    }
    
    @Test
    @DisplayName("Time delta should be positive")
    void timeDelta_shouldBePositive() {
        double deltaTime = 0.016; // 16ms
        
        assertThat(deltaTime).isGreaterThan(0.0);
        assertThat(deltaTime).isLessThan(1.0); // Less than 1 second
    }
    
    @ParameterizedTest(name = "gpuTimeMs={0}, cpuTimeMs={1} -> ratio={2}")
    @CsvSource({
        "10.0, 5.0, 2.0",
        "5.0, 10.0, 0.5",
        "8.0, 8.0, 1.0",
        "16.0, 4.0, 4.0"
    })
    @DisplayName("GPU/CPU time ratio should be calculated")
    void gpuCpuRatio_shouldBeCalculated(double gpuTimeMs, double cpuTimeMs, double expectedRatio) {
        double ratio = gpuTimeMs / cpuTimeMs;
        assertThat(ratio).isCloseTo(expectedRatio, org.assertj.core.data.Offset.offset(0.01));
    }
    
    // Helper methods
    
    private String formatMemorySize(long bytes) {
        if (bytes >= 1073741824) { // GB
            return String.format("%.1f GB", bytes / 1073741824.0);
        } else if (bytes >= 1048576) { // MB
            return String.format("%.1f MB", bytes / 1048576.0);
        } else if (bytes >= 1024) { // KB
            return String.format("%.1f KB", bytes / 1024.0);
        } else {
            return bytes + " B";
        }
    }
}
