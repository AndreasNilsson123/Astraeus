package com.astraeus.native_api;

import java.lang.foreign.*;
import java.lang.invoke.VarHandle;

/**
 * Wrapper for FrameStats struct from native engine.
 * Provides safe, read-only access to frame statistics.
 */
public class FrameStatsView {
    private final long frameNumber;
    private final double deltaTimeMs;
    private final double renderTimeMs;
    private final int drawCalls;
    private final int triangleCount;
    private final int entityCount;
    
    public FrameStatsView(MemorySegment structSegment) {
        VarHandle frameNumberHandle = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("frame_number"));
        VarHandle deltaTimeMsHandle = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("delta_time_ms"));
        VarHandle renderTimeMsHandle = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("render_time_ms"));
        VarHandle drawCallsHandle = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("draw_calls"));
        VarHandle triangleCountHandle = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("triangle_count"));
        VarHandle entityCountHandle = EngineBindings.FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("entity_count"));
        
        this.frameNumber = (long) frameNumberHandle.get(structSegment, 0L);
        this.deltaTimeMs = (double) deltaTimeMsHandle.get(structSegment, 0L);
        this.renderTimeMs = (double) renderTimeMsHandle.get(structSegment, 0L);
        this.drawCalls = (int) drawCallsHandle.get(structSegment, 0L);
        this.triangleCount = (int) triangleCountHandle.get(structSegment, 0L);
        this.entityCount = (int) entityCountHandle.get(structSegment, 0L);
    }
    
    public long getFrameNumber() { return frameNumber; }
    public double getDeltaTimeMs() { return deltaTimeMs; }
    public double getRenderTimeMs() { return renderTimeMs; }
    public int getDrawCalls() { return drawCalls; }
    public int getTriangleCount() { return triangleCount; }
    public int getEntityCount() { return entityCount; }
    
    /**
     * Calculate frames per second from delta time.
     */
    public double getFPS() {
        return deltaTimeMs > 0 ? 1000.0 / deltaTimeMs : 0.0;
    }
    
    @Override
    public String toString() {
        return String.format("FrameStats{frame=%d, fps=%.1f, deltaMs=%.2f, renderMs=%.2f, draws=%d, tris=%d, entities=%d}",
            frameNumber, getFPS(), deltaTimeMs, renderTimeMs, drawCalls, triangleCount, entityCount);
    }
}
