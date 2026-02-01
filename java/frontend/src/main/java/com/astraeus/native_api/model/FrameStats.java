package com.astraeus.native_api.model;

import com.astraeus.native_api.EngineBindings;

import java.lang.foreign.MemoryLayout;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;

/**
 * Telemetry frame statistics from the native engine.
 * 
 * <p>Provides an immutable snapshot of a single frame's performance metrics,
 * including CPU/GPU timings, draw calls, triangle counts, and render pass counts.</p>
 * 
 * <p><b>Thread Safety:</b> This class is immutable and thread-safe after construction.</p>
 * 
 * <p><b>Usage:</b></p>
 * <pre>{@code
 * NativeEngine engine = ...;
 * engine.enableTelemetry(true);
 * 
 * // Get current frame stats
 * FrameStats stats = engine.getTelemetryStats();
 * System.out.println("FPS: " + stats.getFPS());
 * System.out.println("CPU: " + stats.getCpuTimeMs() + "ms");
 * System.out.println("Draw calls: " + stats.getDrawCalls());
 * 
 * // Get historical stats
 * List<FrameStats> history = engine.getTelemetryHistory(100);
 * }</pre>
 * 
 * @see com.astraeus.native_api.NativeEngine#enableTelemetry(boolean)
 * @see com.astraeus.native_api.NativeEngine#getTelemetryStats()
 * @see com.astraeus.native_api.NativeEngine#getTelemetryHistory(int)
 */
public class FrameStats {
    private final long frameNumber;
    private final double cpuTimeMs;
    private final double gpuTimeMs;
    private final double totalTimeMs;
    private final int drawCalls;
    private final int triangleCount;
    private final int passCount;
    
    /**
     * Create FrameStats from a native TelemetryFrameStats struct.
     * 
     * @param structSegment Memory segment containing the native struct
     * @throws NullPointerException if structSegment is null
     */
    public FrameStats(MemorySegment structSegment) {
        VarHandle frameNumberHandle = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("frame_number"));
        VarHandle cpuTimeHandle = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("cpu_time_ms"));
        VarHandle gpuTimeHandle = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("gpu_time_ms"));
        VarHandle totalTimeHandle = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("total_time_ms"));
        VarHandle drawCallsHandle = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("draw_calls"));
        VarHandle triangleCountHandle = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("triangle_count"));
        VarHandle passCountHandle = EngineBindings.TELEMETRY_FRAME_STATS_LAYOUT.varHandle(
            MemoryLayout.PathElement.groupElement("pass_count"));
        
        this.frameNumber = (long) frameNumberHandle.get(structSegment, 0L);
        this.cpuTimeMs = (double) cpuTimeHandle.get(structSegment, 0L);
        this.gpuTimeMs = (double) gpuTimeHandle.get(structSegment, 0L);
        this.totalTimeMs = (double) totalTimeHandle.get(structSegment, 0L);
        this.drawCalls = (int) drawCallsHandle.get(structSegment, 0L);
        this.triangleCount = (int) triangleCountHandle.get(structSegment, 0L);
        this.passCount = (byte) passCountHandle.get(structSegment, 0L);
    }
    
    /**
     * Get the frame number.
     * 
     * <p>This is a monotonically increasing counter that starts at 0 when
     * the engine is created.</p>
     * 
     * @return Frame number
     */
    public long getFrameNumber() { 
        return frameNumber; 
    }

    /**
     * Get the CPU time for this frame in milliseconds.
     * 
     * <p>This includes all CPU-side work: scene updates, render graph execution,
     * command submission, etc.</p>
     * 
     * @return CPU time in milliseconds
     */
    public double getCpuTimeMs() { 
        return cpuTimeMs; 
    }

    /**
     * Get the GPU time for this frame in milliseconds.
     * 
     * <p>This represents the actual GPU execution time, measured via GPU queries
     * if available. May be 0 if GPU timing is not supported or disabled.</p>
     * 
     * @return GPU time in milliseconds (0 if unavailable)
     */
    public double getGpuTimeMs() { 
        return gpuTimeMs; 
    }

    /**
     * Get the total frame time in milliseconds.
     * 
     * <p>This is the wall-clock time from beginFrame() to endFrame().</p>
     * 
     * @return Total frame time in milliseconds
     */
    public double getTotalTimeMs() { 
        return totalTimeMs; 
    }

    /**
     * Get the number of draw calls submitted in this frame.
     * 
     * <p>This includes all draw commands across all render passes.</p>
     * 
     * @return Number of draw calls
     */
    public int getDrawCalls() { 
        return drawCalls; 
    }

    /**
     * Get the number of triangles rendered in this frame.
     * 
     * <p>This is the sum of triangles across all draw calls.</p>
     * 
     * @return Number of triangles
     */
    public int getTriangleCount() { 
        return triangleCount; 
    }

    /**
     * Get the number of render passes executed in this frame.
     * 
     * @return Number of render passes
     */
    public int getPassCount() { 
        return passCount; 
    }
    
    /**
     * Calculate frames per second from total frame time.
     * 
     * @return FPS (0 if totalTimeMs is 0)
     */
    public double getFPS() {
        return totalTimeMs > 0 ? 1000.0 / totalTimeMs : 0.0;
    }

    @Override
    public String toString() {
        return String.format("FrameStats{frame=%d, fps=%.1f, cpu=%.2fms, gpu=%.2fms, " +
                           "draws=%d, tris=%d, passes=%d}",
                           frameNumber, getFPS(), cpuTimeMs, gpuTimeMs, 
                           drawCalls, triangleCount, passCount);
    }
}
