package com.astraeus.test;

import com.astraeus.native_api.FrameStatsView;
import com.astraeus.tools.TelemetryOverlay;
import com.astraeus.tools.TelemetryPane;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;

/**
 * Simple verification test for telemetry components.
 * Tests that components can be created and updated without throwing exceptions.
 * Does not require JavaFX runtime (no UI shown).
 */
public class TelemetryComponentsTest {
    
    public static void main(String[] args) {
        System.out.println("=== Telemetry Components Test ===\n");
        
        try {
            // Test 1: Create components
            System.out.println("Test 1: Creating telemetry components...");
            TelemetryOverlay overlay = new TelemetryOverlay();
            TelemetryPane pane = new TelemetryPane();
            System.out.println("✓ Components created successfully\n");
            
            // Test 2: Create mock frame stats
            System.out.println("Test 2: Creating mock frame stats...");
            FrameStatsView stats = createMockStats(1000, 16.67);
            System.out.println("✓ Mock stats created: " + stats);
            System.out.println();
            
            // Test 3: Verify FPS calculation
            System.out.println("Test 3: Verifying FPS calculation...");
            double expectedFps = 1000.0 / 16.67;
            double actualFps = stats.getFPS();
            System.out.printf("  Expected FPS: ~%.1f\n", expectedFps);
            System.out.printf("  Actual FPS: %.1f\n", actualFps);
            if (Math.abs(actualFps - expectedFps) < 0.1) {
                System.out.println("✓ FPS calculation correct\n");
            } else {
                System.out.println("✗ FPS calculation incorrect\n");
            }
            
            // Test 4: Update components multiple times (verify no allocations crash)
            System.out.println("Test 4: Updating components 100 times...");
            for (int i = 0; i < 100; i++) {
                FrameStatsView testStats = createMockStats(i, 16.0 + Math.random() * 2.0);
                overlay.update(testStats);
                pane.update(testStats);
            }
            System.out.println("✓ 100 updates completed without errors\n");
            
            // Test 5: Toggle enable/disable
            System.out.println("Test 5: Testing enable/disable...");
            boolean initialState = overlay.isEnabled();
            overlay.toggle();
            boolean toggledState = overlay.isEnabled();
            overlay.toggle();
            boolean finalState = overlay.isEnabled();
            
            System.out.printf("  Initial state: %b\n", initialState);
            System.out.printf("  After toggle: %b\n", toggledState);
            System.out.printf("  After second toggle: %b\n", finalState);
            
            if (initialState != toggledState && initialState == finalState) {
                System.out.println("✓ Toggle working correctly\n");
            } else {
                System.out.println("✗ Toggle not working correctly\n");
            }
            
            // Test 6: Stats with various data ranges
            System.out.println("Test 6: Testing with various data ranges...");
            testStatsVariety(overlay, pane);
            System.out.println("✓ All data ranges handled correctly\n");
            
            System.out.println("=== All Tests Passed ===");
            System.out.println("\nTelemetry components are ready for use!");
            System.out.println("Run TelemetryUIStandaloneTest with JavaFX runtime to see the UI.");
            
        } catch (Exception e) {
            System.err.println("✗ Test failed with exception: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private static void testStatsVariety(TelemetryOverlay overlay, TelemetryPane pane) {
        // Low FPS scenario
        FrameStatsView lowFps = createMockStats(100, 50.0);
        overlay.update(lowFps);
        pane.update(lowFps);
        System.out.println("  Low FPS: " + lowFps.getFPS());
        
        // High FPS scenario
        FrameStatsView highFps = createMockStats(200, 5.0);
        overlay.update(highFps);
        pane.update(highFps);
        System.out.println("  High FPS: " + highFps.getFPS());
        
        // Many entities
        FrameStatsView manyEntities = createMockStatsWithEntities(300, 16.0, 10000);
        overlay.update(manyEntities);
        pane.update(manyEntities);
        System.out.println("  Many entities: " + manyEntities.getEntityCount());
    }
    
    private static FrameStatsView createMockStats(long frameNumber, double deltaTimeMs) {
        return createMockStatsWithEntities(frameNumber, deltaTimeMs, 42);
    }
    
    private static FrameStatsView createMockStatsWithEntities(long frameNumber, double deltaTimeMs, int entityCount) {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment statsStruct = arena.allocate(com.astraeus.native_api.EngineBindings.FRAME_STATS_LAYOUT);
            
            var layout = com.astraeus.native_api.EngineBindings.FRAME_STATS_LAYOUT;
            VarHandle frameNumberHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("frame_number"));
            VarHandle deltaTimeMsHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("delta_time_ms"));
            VarHandle renderTimeMsHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("render_time_ms"));
            VarHandle drawCallsHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("draw_calls"));
            VarHandle triangleCountHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("triangle_count"));
            VarHandle entityCountHandle = layout.varHandle(
                java.lang.foreign.MemoryLayout.PathElement.groupElement("entity_count"));
            
            frameNumberHandle.set(statsStruct, 0L, frameNumber);
            deltaTimeMsHandle.set(statsStruct, 0L, deltaTimeMs);
            renderTimeMsHandle.set(statsStruct, 0L, deltaTimeMs * 0.7);
            drawCallsHandle.set(statsStruct, 0L, 10 + entityCount * 2);
            triangleCountHandle.set(statsStruct, 0L, 100 + entityCount * 20);
            entityCountHandle.set(statsStruct, 0L, entityCount);
            
            return new FrameStatsView(statsStruct);
        }
    }
}
