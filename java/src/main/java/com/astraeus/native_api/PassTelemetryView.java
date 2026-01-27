package com.astraeus.native_api;

import java.lang.foreign.MemorySegment;
import java.lang.foreign.MemoryLayout;
import java.lang.invoke.VarHandle;
import java.nio.charset.StandardCharsets;

/**
 * Safe wrapper for reading PassTelemetry struct from native memory.
 * Provides efficient access to per-pass timing data.
 * 
 * This class is designed to be reused to avoid per-frame allocations.
 */
public class PassTelemetryView {
    
    // VarHandle for efficient field access (initialized once)
    private static final VarHandle DURATION_HANDLE;
    private static final long PASS_NAME_OFFSET;
    
    static {
        DURATION_HANDLE = EngineBindings.PASS_TELEMETRY_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("duration_ms"));
        PASS_NAME_OFFSET = EngineBindings.PASS_TELEMETRY_LAYOUT.byteOffset(
            MemoryLayout.PathElement.groupElement("pass_name"));
    }
    
    // Cached values (updated on refresh)
    private String passName;
    private double durationMs;
    
    /**
     * Create a new PassTelemetryView.
     * Call refresh() to populate with current data.
     */
    public PassTelemetryView() {
        this.passName = "";
        this.durationMs = 0.0;
    }
    
    /**
     * Refresh this view with data from the provided PassTelemetry struct.
     * This method minimizes allocations by reusing string data where possible.
     * 
     * @param telemetrySegment MemorySegment pointing to PassTelemetry struct
     */
    public void refresh(MemorySegment telemetrySegment) {
        // Read duration
        this.durationMs = (double) DURATION_HANDLE.get(telemetrySegment, 0L);
        
        // Read pass name (null-terminated C string)
        // Extract the char[64] array as bytes
        byte[] nameBytes = new byte[64];
        MemorySegment nameSegment = telemetrySegment.asSlice(PASS_NAME_OFFSET, 64);
        for (int i = 0; i < 64; i++) {
            nameBytes[i] = nameSegment.get(java.lang.foreign.ValueLayout.JAVA_BYTE, i);
            if (nameBytes[i] == 0) {
                // Null terminator found, create string up to this point
                this.passName = new String(nameBytes, 0, i, StandardCharsets.UTF_8);
                return;
            }
        }
        // No null terminator found (shouldn't happen with properly formatted C strings)
        this.passName = new String(nameBytes, StandardCharsets.UTF_8).trim();
    }
    
    // Getters for cached values
    
    public String getPassName() {
        return passName;
    }
    
    public double getDurationMs() {
        return durationMs;
    }
    
    /**
     * Get duration as percentage of total frame time.
     * @param totalFrameTimeMs Total frame time in milliseconds
     * @return Percentage (0-100)
     */
    public double getPercentage(double totalFrameTimeMs) {
        return totalFrameTimeMs > 0.0 ? (durationMs / totalFrameTimeMs) * 100.0 : 0.0;
    }
    
    @Override
    public String toString() {
        return String.format("PassTelemetry[%s: %.3fms]", passName, durationMs);
    }
}
