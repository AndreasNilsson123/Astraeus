#ifndef ASTRAEUS_TELEMETRY_HPP
#define ASTRAEUS_TELEMETRY_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <array>

namespace astraeus {

/**
 * High-precision timer for measuring frame and pass timing.
 * Uses std::chrono::high_resolution_clock for CPU timing.
 */
class Timer {
public:
    Timer() : start_time_(clock::now()) {}

    void reset() {
        start_time_ = clock::now();
    }

    double elapsed_ms() const {
        auto now = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_);
        return duration.count() / 1000.0;
    }

private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;
    time_point start_time_;
};

/**
 * Per-render-pass telemetry data.
 */
struct PassTelemetryData {
    std::string name;
    double duration_ms;
    
    PassTelemetryData() : duration_ms(0.0) {}
    PassTelemetryData(const std::string& n, double d) : name(n), duration_ms(d) {}
};

/**
 * Frame-level telemetry data.
 */
struct FrameTelemetryData {
    double cpu_time_ms;
    double gpu_time_ms;
    uint32_t draw_calls;
    uint32_t triangle_count;
    std::vector<PassTelemetryData> pass_timings;
    
    FrameTelemetryData() 
        : cpu_time_ms(0.0)
        , gpu_time_ms(0.0)
        , draw_calls(0)
        , triangle_count(0) 
    {}
};

/**
 * Telemetry system for tracking frame-level and per-pass performance metrics.
 * 
 * Features:
 * - Frame-level counters (CPU time, GPU time, draw calls, triangles)
 * - Per-pass timing information
 * - Ring-buffer storage for historical data (last N frames)
 * - Runtime enable/disable with zero overhead when disabled
 * - ≤1-2% overhead when enabled
 */
class TelemetrySystem {
public:
    static constexpr uint32_t DEFAULT_HISTORY_SIZE = 120;  // Store last 120 frames
    
    explicit TelemetrySystem(uint32_t history_size = DEFAULT_HISTORY_SIZE);
    ~TelemetrySystem() = default;

    // Non-copyable
    TelemetrySystem(const TelemetrySystem&) = delete;
    TelemetrySystem& operator=(const TelemetrySystem&) = delete;

    /**
     * Enable or disable telemetry collection.
     * When disabled, all telemetry operations have zero cost.
     */
    void set_enabled(bool enabled) { enabled_ = enabled; }
    
    /**
     * Check if telemetry is currently enabled.
     */
    bool is_enabled() const { return enabled_; }

    /**
     * Begin a new frame. Call at the start of frame rendering.
     */
    void begin_frame();

    /**
     * End the current frame and store telemetry data.
     * @param gpu_time_ms GPU frame time in milliseconds (from GPU queries)
     * @param draw_calls Number of draw calls this frame
     * @param triangle_count Number of triangles rendered this frame
     */
    void end_frame(double gpu_time_ms, uint32_t draw_calls, uint32_t triangle_count);

    /**
     * Begin timing a render pass.
     * @param pass_name Name of the render pass
     */
    void begin_pass(const std::string& pass_name);

    /**
     * End timing the current render pass.
     */
    void end_pass();

    /**
     * Get the current frame's telemetry data.
     */
    const FrameTelemetryData& get_current_frame() const { return current_frame_; }

    /**
     * Get telemetry data for a specific historical frame.
     * @param frames_ago Number of frames back (0 = current frame, 1 = previous frame, etc.)
     * @return Pointer to frame data, or nullptr if not available
     */
    const FrameTelemetryData* get_frame(uint32_t frames_ago) const;

    /**
     * Get the number of passes in the current frame.
     */
    uint32_t get_pass_count() const;

    /**
     * Get telemetry for a specific pass by index.
     * @param pass_index Index of the pass (0 to get_pass_count()-1)
     * @return Pointer to pass data, or nullptr if invalid index
     */
    const PassTelemetryData* get_pass_telemetry(uint32_t pass_index) const;

    /**
     * Clear all historical data.
     */
    void clear_history();

private:
    bool enabled_;
    uint32_t history_size_;
    
    // Current frame being recorded
    FrameTelemetryData current_frame_;
    Timer frame_timer_;
    
    // Current pass being recorded
    Timer pass_timer_;
    std::string current_pass_name_;
    bool pass_in_progress_;
    
    // Ring buffer for historical data
    std::vector<FrameTelemetryData> frame_history_;
    uint32_t history_write_index_;
    uint32_t history_count_;  // Number of frames stored (up to history_size_)
};

} // namespace astraeus

#endif // ASTRAEUS_TELEMETRY_HPP
