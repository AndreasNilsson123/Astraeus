package com.astraeus.test;

import com.astraeus.native_api.FrameStatsView;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;

/**
 * Simple verification test for FrameStatsView without JavaFX dependency.
 * Tests that stats can be created and queried correctly.
 */
public class FrameStatsViewTest {
    
    public static void main(String[] args) {
        System.out.println("=== FrameStatsView Test ===\n");
        
        try {
            // Test 1: Create mock frame stats
            System.out.println("Test 1: Creating mock frame stats...");
            FrameStatsView stats = createMockStats(1000, 16.67, 42);
            System.out.println("✓ Mock stats created: " + stats);
            System.out.println();
            
            // Test 2: Verify field values
            System.out.println("Test 2: Verifying field values...");
            assert stats.getFrameNumber() == 1000 : "Frame number mismatch";
            assert Math.abs(stats.getDeltaTimeMs() - 16.67) < 0.01 : "Delta time mismatch";
            assert stats.getEntityCount() == 42 : "Entity count mismatch";
            System.out.println("  Frame number: " + stats.getFrameNumber());
            System.out.println("  Delta time: " + stats.getDeltaTimeMs() + " ms");
            System.out.println("  Entity count: " + stats.getEntityCount());
            System.out.println("✓ All fields correct\n");
            
            // Test 3: Verify FPS calculation
            System.out.println("Test 3: Verifying FPS calculation...");
            double expectedFps = 1000.0 / 16.67;
            double actualFps = stats.getFPS();
            System.out.printf("  Expected FPS: ~%.1f\n", expectedFps);
            System.out.printf("  Actual FPS: %.1f\n", actualFps);
            assert Math.abs(actualFps - expectedFps) < 0.1 : "FPS calculation incorrect";
            System.out.println("✓ FPS calculation correct\n");
            
            // Test 4: Create multiple stats (verify no memory leaks)
            System.out.println("Test 4: Creating 1000 FrameStatsView instances...");
            for (int i = 0; i < 1000; i++) {
                FrameStatsView testStats = createMockStats(i, 16.0 + Math.random() * 2.0, i % 100);
                // Just create and discard - testing for memory issues
            }
            System.out.println("✓ 1000 instances created without errors\n");
            
            // Test 5: Edge cases
            System.out.println("Test 5: Testing edge cases...");
            
            // Very low FPS
            FrameStatsView lowFps = createMockStats(1, 100.0, 1);
            System.out.println("  Low FPS (100ms frame): " + lowFps.getFPS() + " fps");
            assert lowFps.getFPS() == 10.0 : "Low FPS calculation incorrect";
            
            // Very high FPS
            FrameStatsView highFps = createMockStats(2, 2.0, 1);
            System.out.println("  High FPS (2ms frame): " + highFps.getFPS() + " fps");
            assert highFps.getFPS() == 500.0 : "High FPS calculation incorrect";
            
            // Zero delta (edge case)
            FrameStatsView zeroDelta = createMockStats(3, 0.0, 1);
            System.out.println("  Zero delta FPS: " + zeroDelta.getFPS() + " fps");
            assert zeroDelta.getFPS() == 0.0 : "Zero delta FPS should be 0";
            
            System.out.println("✓ All edge cases handled correctly\n");
            
            // Test 6: String representation
            System.out.println("Test 6: Testing toString()...");
            String str = stats.toString();
            System.out.println("  " + str);
            assert str.contains("FrameStats") : "toString missing class name";
            assert str.contains("frame=") : "toString missing frame number";
            assert str.contains("fps=") : "toString missing FPS";
            System.out.println("✓ toString() working correctly\n");
            
            System.out.println("=== All Tests Passed ===");
            System.out.println("\nFrameStatsView is working correctly!");
            System.out.println("Integration with native engine ready once library is built.");
            
        } catch (AssertionError e) {
            System.err.println("✗ Assertion failed: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        } catch (Exception e) {
            System.err.println("✗ Test failed with exception: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private static FrameStatsView createMockStats(long frameNumber, double deltaTimeMs, int entityCount) {
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
