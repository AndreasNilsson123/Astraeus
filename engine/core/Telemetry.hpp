#ifndef ASTRAEUS_TELEMETRY_HPP
#define ASTRAEUS_TELEMETRY_HPP

#include <cstdint>
#include <cstring>
#include <chrono>
#include <array>
#include <string>

namespace astraeus {

/**
 * High-performance telemetry system for tracking frame-level and pass-level metrics.
 * 
 * Design Goals:
 * - Zero overhead when disabled (compile-time checks)
 * - ≤ 1-2% overhead when enabled
 * - No dynamic allocations per frame
 * - Ring buffer for historical data
 * - Per-pass timing support
 */
class Telemetry {
public:
    // Maximum number of render passes we can track
    static constexpr uint32_t MAX_PASSES = 16;
    
    // Ring buffer size (300 frames = ~5 seconds at 60 FPS)
    static constexpr uint32_t HISTORY_SIZE = 300;
    
    // Per-frame statistics (compact POD for cache efficiency)
    struct FrameStats {
        uint64_t frame_number = 0;
        double cpu_time_ms = 0.0;
        double gpu_time_ms = 0.0;      // Placeholder for now (requires GPU queries)
        double total_time_ms = 0.0;
        uint32_t draw_calls = 0;
        uint32_t triangle_count = 0;
        uint8_t pass_count = 0;        // Number of active passes this frame
        uint8_t _padding[3] = {0};     // Explicit padding for alignment
    };
    
    // Per-pass timing information
    struct PassTiming {
        char name[32] = {0};           // Pass name (null-terminated)
        double duration_ms = 0.0;
        bool active = false;
        uint8_t _padding[7] = {0};     // Explicit padding for alignment
    };
    
    Telemetry();
    ~Telemetry() = default;
    
    // Non-copyable, movable
    Telemetry(const Telemetry&) = delete;
    Telemetry& operator=(const Telemetry&) = delete;
    Telemetry(Telemetry&&) noexcept = default;
    Telemetry& operator=(Telemetry&&) noexcept = default;
    
    /**
     * Enable or disable telemetry collection.
     * When disabled, all timing operations become no-ops.
     */
    inline void set_enabled(bool enabled) { enabled_ = enabled; }
    
    /**
     * Check if telemetry is enabled.
     */
    inline bool is_enabled() const { return enabled_; }
    
    /**
     * Begin a new frame and start timing.
     */
    inline void begin_frame(uint64_t frame_number);
    
    /**
     * End the current frame and record statistics.
     */
    inline void end_frame(uint32_t draw_calls, uint32_t triangle_count);
    
    /**
     * Begin timing a render pass.
     * Returns an index that should be passed to end_pass.
     */
    inline uint32_t begin_pass(const char* name);
    
    /**
     * End timing a render pass.
     */
    inline void end_pass(uint32_t pass_index);
    
    /**
     * Get the current frame statistics.
     */
    inline const FrameStats& get_current_stats() const { return current_stats_; }
    
    /**
     * Get historical frame statistics.
     * @param out_buffer Output buffer for frame stats
     * @param max_frames Maximum number of frames to retrieve
     * @return Number of frames actually written
     */
    uint32_t get_history(FrameStats* out_buffer, uint32_t max_frames) const;
    
    /**
     * Get the number of render passes in the current frame.
     */
    inline uint32_t get_pass_count() const { return current_stats_.pass_count; }
    
    /**
     * Get timing information for a specific pass.
     * @param pass_index Index of the pass (0 to pass_count-1)
     * @return Pointer to pass timing, or nullptr if index is invalid
     */
    inline const PassTiming* get_pass_timing(uint32_t pass_index) const;
    
    /**
     * Reset all telemetry data.
     */
    void reset();
    
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    
    bool enabled_;
    
    // Current frame state
    FrameStats current_stats_;
    TimePoint frame_start_time_;
    
    // Per-pass tracking
    std::array<PassTiming, MAX_PASSES> pass_timings_;
    std::array<TimePoint, MAX_PASSES> pass_start_times_;
    
    // Ring buffer for historical data
    std::array<FrameStats, HISTORY_SIZE> history_;
    uint32_t history_head_;  // Next write position
    uint32_t history_count_; // Number of valid entries (up to HISTORY_SIZE)
};

// =============================================================================
// RAII Timer for automatic pass timing
// =============================================================================

/**
 * RAII-style timer for automatic pass timing.
 * Usage:
 *   {
 *       PassTimer timer(telemetry, "MyPass");
 *       // ... render pass code ...
 *   }  // Automatically records timing on scope exit
 */
class PassTimer {
public:
    inline PassTimer(Telemetry* telemetry, const char* name)
        : telemetry_(telemetry)
        , pass_index_(0)
        , active_(false)
    {
        if (telemetry_ && telemetry_->is_enabled()) {
            pass_index_ = telemetry_->begin_pass(name);
            active_ = true;
        }
    }
    
    inline ~PassTimer() {
        if (active_ && telemetry_) {
            telemetry_->end_pass(pass_index_);
        }
    }
    
    // Non-copyable, non-movable
    PassTimer(const PassTimer&) = delete;
    PassTimer& operator=(const PassTimer&) = delete;
    PassTimer(PassTimer&&) = delete;
    PassTimer& operator=(PassTimer&&) = delete;
    
private:
    Telemetry* telemetry_;
    uint32_t pass_index_;
    bool active_;
};

// =============================================================================
// INLINE IMPLEMENTATIONS
// =============================================================================

inline Telemetry::Telemetry()
    : enabled_(true)
    , current_stats_{}
    , frame_start_time_{}
    , pass_timings_{}
    , pass_start_times_{}
    , history_{}
    , history_head_(0)
    , history_count_(0)
{
}

inline void Telemetry::begin_frame(uint64_t frame_number) {
    if (!enabled_) return;
    
    // Reset current frame stats
    current_stats_ = FrameStats{};
    current_stats_.frame_number = frame_number;
    
    // Reset pass timings
    for (auto& timing : pass_timings_) {
        timing.active = false;
    }
    
    // Start timing
    frame_start_time_ = Clock::now();
}

inline void Telemetry::end_frame(uint32_t draw_calls, uint32_t triangle_count) {
    if (!enabled_) return;
    
    // End timing
    auto frame_end_time = Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        frame_end_time - frame_start_time_
    );
    
    // Record frame statistics
    current_stats_.cpu_time_ms = duration.count() / 1'000'000.0;
    current_stats_.total_time_ms = current_stats_.cpu_time_ms + current_stats_.gpu_time_ms;
    current_stats_.draw_calls = draw_calls;
    current_stats_.triangle_count = triangle_count;
    
    // Store in ring buffer
    history_[history_head_] = current_stats_;
    history_head_ = (history_head_ + 1) % HISTORY_SIZE;
    if (history_count_ < HISTORY_SIZE) {
        history_count_++;
    }
}

inline uint32_t Telemetry::begin_pass(const char* name) {
    if (!enabled_) return 0;
    
    // Find next available pass slot
    uint32_t pass_index = current_stats_.pass_count;
    if (pass_index >= MAX_PASSES) {
        // Too many passes, ignore
        return 0;
    }
    
    // Initialize pass timing
    PassTiming& timing = pass_timings_[pass_index];
    timing.active = true;
    timing.duration_ms = 0.0;
    
    // Copy pass name (safely)
    if (name) {
        std::strncpy(timing.name, name, sizeof(timing.name) - 1);
        timing.name[sizeof(timing.name) - 1] = '\0';
    } else {
        timing.name[0] = '\0';
    }
    
    // Start timing
    pass_start_times_[pass_index] = Clock::now();
    
    current_stats_.pass_count++;
    return pass_index;
}

inline void Telemetry::end_pass(uint32_t pass_index) {
    if (!enabled_) return;
    
    if (pass_index >= MAX_PASSES || !pass_timings_[pass_index].active) {
        return;
    }
    
    // End timing
    auto end_time = Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - pass_start_times_[pass_index]
    );
    
    // Record duration
    pass_timings_[pass_index].duration_ms = duration.count() / 1'000'000.0;
}

inline const Telemetry::PassTiming* Telemetry::get_pass_timing(uint32_t pass_index) const {
    if (pass_index >= MAX_PASSES || !pass_timings_[pass_index].active) {
        return nullptr;
    }
    return &pass_timings_[pass_index];
}

inline uint32_t Telemetry::get_history(FrameStats* out_buffer, uint32_t max_frames) const {
    if (!out_buffer || max_frames == 0) {
        return 0;
    }
    
    // Limit to available frames
    uint32_t frames_to_copy = (max_frames < history_count_) ? max_frames : history_count_;
    
    // Copy from ring buffer (oldest to newest)
    uint32_t start_index = (history_count_ < HISTORY_SIZE) ? 0 : history_head_;
    
    for (uint32_t i = 0; i < frames_to_copy; ++i) {
        uint32_t src_index = (start_index + i) % HISTORY_SIZE;
        out_buffer[i] = history_[src_index];
    }
    
    return frames_to_copy;
}

inline void Telemetry::reset() {
    current_stats_ = FrameStats{};
    
    for (auto& timing : pass_timings_) {
        timing = PassTiming{};
    }
    
    for (auto& stats : history_) {
        stats = FrameStats{};
    }
    
    history_head_ = 0;
    history_count_ = 0;
}

} // namespace astraeus

#endif // ASTRAEUS_TELEMETRY_HPP
