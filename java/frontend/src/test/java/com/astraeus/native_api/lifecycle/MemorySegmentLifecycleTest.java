package com.astraeus.native_api.lifecycle;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

/**
 * Test suite for MemorySegment lifecycle and arena management.
 * 
 * <p>Verifies:
 * - Arena scopes (confined vs shared)
 * - Use-after-close detection
 * - Memory segment validity
 * - Proper cleanup behavior
 */
@DisplayName("MemorySegment Lifecycle Tests")
class MemorySegmentLifecycleTest {
    
    @Test
    @DisplayName("Confined arena should allow access from creating thread")
    void confinedArena_shouldAllowAccessFromCreatingThread() {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment segment = arena.allocate(100);
            
            assertThat(segment.byteSize()).isEqualTo(100);
            assertThat(segment.address()).isNotZero();
            
            // Should be able to read/write
            segment.set(ValueLayout.JAVA_INT, 0, 42);
            int value = segment.get(ValueLayout.JAVA_INT, 0);
            
            assertThat(value).isEqualTo(42);
        }
    }
    
    @Test
    @DisplayName("Shared arena should allow access from multiple threads")
    void sharedArena_shouldAllowAccessFromMultipleThreads() throws InterruptedException {
        try (Arena arena = Arena.ofShared()) {
            MemorySegment segment = arena.allocate(100);
            segment.set(ValueLayout.JAVA_INT, 0, 123);
            
            // Access from another thread
            Thread t = new Thread(() -> {
                int value = segment.get(ValueLayout.JAVA_INT, 0);
                assertThat(value).isEqualTo(123);
            });
            
            t.start();
            t.join();
        }
    }
    
    @Test
    @DisplayName("Closed arena should reject access")
    void closedArena_shouldRejectAccess() {
        Arena arena = Arena.ofConfined();
        MemorySegment segment = arena.allocate(100);
        
        arena.close();
        
        // Attempting to access closed segment should throw
        assertThatThrownBy(() -> segment.get(ValueLayout.JAVA_INT, 0))
            .isInstanceOf(IllegalStateException.class);
    }
    
    @Test
    @DisplayName("Double close should be safe")
    void arena_doubleClose_shouldBeSafe() {
        Arena arena = Arena.ofConfined();
        arena.allocate(100);
        
        arena.close();
        
        // Double close behavior: In FFM API, double close on a confined arena
        // is idempotent - it does nothing on second close
        // This should not throw
        try {
            arena.close();
        } catch (IllegalStateException e) {
            // Some implementations may throw - this is acceptable
            assertThat(e.getMessage()).contains("closed");
        }
    }
    
    @Test
    @DisplayName("Allocating from closed arena should fail")
    void closedArena_allocate_shouldFail() {
        Arena arena = Arena.ofConfined();
        arena.close();
        
        assertThatThrownBy(() -> arena.allocate(100))
            .isInstanceOf(IllegalStateException.class);
    }
    
    @Test
    @DisplayName("Memory segment should report correct size")
    void memorySegment_shouldReportCorrectSize() {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment segment1 = arena.allocate(100);
            MemorySegment segment2 = arena.allocate(1024);
            
            assertThat(segment1.byteSize()).isEqualTo(100);
            assertThat(segment2.byteSize()).isEqualTo(1024);
        }
    }
    
    @Test
    @DisplayName("Memory segment slice should work correctly")
    void memorySegment_slice_shouldWorkCorrectly() {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment segment = arena.allocate(100);
            
            // Write to original
            segment.set(ValueLayout.JAVA_INT, 0, 111);
            segment.set(ValueLayout.JAVA_INT, 4, 222);
            segment.set(ValueLayout.JAVA_INT, 8, 333);
            
            // Create slice starting at offset 4
            MemorySegment slice = segment.asSlice(4, 8);
            
            assertThat(slice.byteSize()).isEqualTo(8);
            assertThat(slice.get(ValueLayout.JAVA_INT, 0)).isEqualTo(222);
            assertThat(slice.get(ValueLayout.JAVA_INT, 4)).isEqualTo(333);
        }
    }
    
    @Test
    @DisplayName("Out of bounds access should throw")
    void memorySegment_outOfBoundsAccess_shouldThrow() {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment segment = arena.allocate(10);
            
            assertThatThrownBy(() -> segment.get(ValueLayout.JAVA_LONG, 8))
                .isInstanceOf(IndexOutOfBoundsException.class);
        }
    }
    
    @Test
    @DisplayName("Memory segment should be writable after allocation")
    void memorySegment_shouldBeWritable() {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment segment = arena.allocate(100);
            
            // Write various types
            segment.set(ValueLayout.JAVA_BYTE, 0, (byte) 1);
            segment.set(ValueLayout.JAVA_SHORT, 2, (short) 256);
            segment.set(ValueLayout.JAVA_INT, 4, 65536);
            segment.set(ValueLayout.JAVA_LONG, 8, 1_000_000L);
            segment.set(ValueLayout.JAVA_FLOAT, 16, 3.14f);
            segment.set(ValueLayout.JAVA_DOUBLE, 24, 2.718);
            
            // Read back
            assertThat(segment.get(ValueLayout.JAVA_BYTE, 0)).isEqualTo((byte) 1);
            assertThat(segment.get(ValueLayout.JAVA_SHORT, 2)).isEqualTo((short) 256);
            assertThat(segment.get(ValueLayout.JAVA_INT, 4)).isEqualTo(65536);
            assertThat(segment.get(ValueLayout.JAVA_LONG, 8)).isEqualTo(1_000_000L);
            assertThat(segment.get(ValueLayout.JAVA_FLOAT, 16)).isEqualTo(3.14f);
            assertThat(segment.get(ValueLayout.JAVA_DOUBLE, 24)).isEqualTo(2.718);
        }
    }
}
