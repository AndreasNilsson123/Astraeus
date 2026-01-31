package com.astraeus.native_api.model;

/**
 * Render pass timing information.
 * 
 * <p>Provides timing data for individual render passes in the render graph.
 * This is useful for profiling and identifying performance bottlenecks.</p>
 * 
 * <p><b>Thread Safety:</b> This class is immutable and thread-safe after construction.</p>
 * 
 * <p><b>Usage:</b></p>
 * <pre>{@code
 * NativeEngine engine = ...;
 * int passCount = engine.getPassCount();
 * 
 * for (int i = 0; i < passCount; i++) {
 *     PassTiming timing = engine.getPassTiming(i);
 *     if (timing != null) {
 *         System.out.println(timing.getName() + ": " + timing.getTimeMs() + "ms");
 *     }
 * }
 * }</pre>
 * 
 * @see com.astraeus.native_api.NativeEngine#getPassCount()
 * @see com.astraeus.native_api.NativeEngine#getPassTiming(int)
 */
public class PassTiming {
    private final String name;
    private final double timeMs;
    
    /**
     * Create a PassTiming instance.
     * 
     * @param name The name of the render pass (e.g., "GridPass", "MeshPass", "PickingPass")
     * @param timeMs The execution time in milliseconds
     * @throws NullPointerException if name is null
     */
    public PassTiming(String name, double timeMs) {
        this.name = name;
        this.timeMs = timeMs;
    }
    
    /**
     * Get the name of the render pass.
     * 
     * @return Pass name (never null)
     */
    public String getName() { 
        return name; 
    }

    /**
     * Get the execution time of this pass in milliseconds.
     * 
     * <p>This represents the GPU or CPU time spent executing this specific
     * render pass, depending on the engine's timing method.</p>
     * 
     * @return Time in milliseconds
     */
    public double getTimeMs() { 
        return timeMs; 
    }

    /**
     * Calculate the percentage of frame time consumed by this pass.
     * 
     * @param totalFrameTimeMs The total frame time
     * @return Percentage (0-100)
     */
    public double getPercentage(double totalFrameTimeMs) {
        return totalFrameTimeMs > 0 ? (timeMs / totalFrameTimeMs) * 100.0 : 0.0;
    }

    @Override
    public String toString() {
        return String.format("PassTiming{name='%s', time=%.3fms}", name, timeMs);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        PassTiming that = (PassTiming) o;
        return Double.compare(that.timeMs, timeMs) == 0 && name.equals(that.name);
    }

    @Override
    public int hashCode() {
        int result = name.hashCode();
        long temp = Double.doubleToLongBits(timeMs);
        result = 31 * result + (int) (temp ^ (temp >>> 32));
        return result;
    }
}
