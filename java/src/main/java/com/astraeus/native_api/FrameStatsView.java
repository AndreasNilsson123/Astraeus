package com.astraeus.native_api;

import java.lang.foreign.MemorySegment;
import java.lang.foreign.MemoryLayout;
import java.lang.invoke.VarHandle;

/**
 * Safe wrapper for reading FrameStats struct from native memory.
 * Provides efficient, allocation-free access to frame statistics.
 * 
 * This class is designed to be reused across frames to avoid per-frame allocations.
 */
public class FrameStatsView {
    
    // VarHandles for efficient field access (initialized once)
    private static final VarHandle FRAME_NUMBER_HANDLE;
    private static final VarHandle DELTA_TIME_HANDLE;
    private static final VarHandle RENDER_TIME_HANDLE;
    private static final VarHandle GPU_TIME_HANDLE;
    private static final VarHandle DRAW_CALLS_HANDLE;
    private static final VarHandle TRIANGLE_COUNT_HANDLE;
    private static final VarHandle ENTITY_COUNT_HANDLE;
    
    static {
        FRAME_NUMBER_HANDLE = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("frame_number"));
        DELTA_TIME_HANDLE = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("delta_time_ms"));
        RENDER_TIME_HANDLE = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("render_time_ms"));
        GPU_TIME_HANDLE = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("gpu_time_ms"));
        DRAW_CALLS_HANDLE = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("draw_calls"));
        TRIANGLE_COUNT_HANDLE = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("triangle_count"));
        ENTITY_COUNT_HANDLE = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("entity_count"));
    }
    
    // Cached values (updated on refresh)
    private long frameNumber;
    private double deltaTimeMs;
    private double renderTimeMs;
    private double gpuTimeMs;
    private int drawCalls;
    private int triangleCount;
    private int entityCount;
    
    /**
     * Create a new FrameStatsView.
     * Call refresh() to populate with current data.
     */
    public FrameStatsView() {
        // Initialize with zeros
        this.frameNumber = 0;
        this.deltaTimeMs = 0.0;
        this.renderTimeMs = 0.0;
        this.gpuTimeMs = 0.0;
        this.drawCalls = 0;
        this.triangleCount = 0;
        this.entityCount = 0;
    }
    
    /**
     * Refresh this view with data from the provided FrameStats struct.
     * This method is allocation-free and can be called every frame.
     * 
     * @param statsSegment MemorySegment pointing to FrameStats struct
     */
    public void refresh(MemorySegment statsSegment) {
        this.frameNumber = (long) FRAME_NUMBER_HANDLE.get(statsSegment, 0L);
        this.deltaTimeMs = (double) DELTA_TIME_HANDLE.get(statsSegment, 0L);
        this.renderTimeMs = (double) RENDER_TIME_HANDLE.get(statsSegment, 0L);
        this.gpuTimeMs = (double) GPU_TIME_HANDLE.get(statsSegment, 0L);
        this.drawCalls = (int) DRAW_CALLS_HANDLE.get(statsSegment, 0L);
        this.triangleCount = (int) TRIANGLE_COUNT_HANDLE.get(statsSegment, 0L);
        this.entityCount = (int) ENTITY_COUNT_HANDLE.get(statsSegment, 0L);
    }
    
    // Getters for cached values
    
    public long getFrameNumber() {
        return frameNumber;
    }
    
    public double getDeltaTimeMs() {
        return deltaTimeMs;
    }
    
    public double getRenderTimeMs() {
        return renderTimeMs;
    }
    
    public double getGpuTimeMs() {
        return gpuTimeMs;
    }
    
    public int getDrawCalls() {
        return drawCalls;
    }
    
    public int getTriangleCount() {
        return triangleCount;
    }
    
    public int getEntityCount() {
        return entityCount;
    }
    
    /**
     * Get frames per second calculated from delta time.
     * @return FPS or 0 if delta time is zero
     */
    public double getFPS() {
        return deltaTimeMs > 0.0 ? 1000.0 / deltaTimeMs : 0.0;
    }
    
    /**
     * Get total frame time (render + other overhead).
     * @return Total frame time in milliseconds
     */
    public double getTotalFrameTimeMs() {
        return deltaTimeMs;
    }
    
    @Override
    public String toString() {
        return String.format(
            "FrameStats[frame=%d, fps=%.1f, cpu=%.2fms, gpu=%.2fms, draws=%d, tris=%d, entities=%d]",
            frameNumber, getFPS(), renderTimeMs, gpuTimeMs, drawCalls, triangleCount, entityCount
        );
    }
}
