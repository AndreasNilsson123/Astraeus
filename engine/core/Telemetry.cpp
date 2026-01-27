#include "Telemetry.hpp"
#include <algorithm>

namespace astraeus {

TelemetrySystem::TelemetrySystem(uint32_t history_size)
    : enabled_(false)  // Disabled by default
    , history_size_(history_size)
    , pass_in_progress_(false)
    , history_write_index_(0)
    , history_count_(0)
{
    // Pre-allocate ring buffer
    frame_history_.resize(history_size_);
}

void TelemetrySystem::begin_frame() {
    if (!enabled_) {
        return;  // Zero cost when disabled
    }

    // Reset current frame data
    current_frame_ = FrameTelemetryData();
    current_frame_.pass_timings.reserve(16);  // Pre-allocate for typical pass count
    
    // Start frame timer
    frame_timer_.reset();
}

void TelemetrySystem::end_frame(double gpu_time_ms, uint32_t draw_calls, uint32_t triangle_count) {
    if (!enabled_) {
        return;  // Zero cost when disabled
    }

    // Record CPU frame time
    current_frame_.cpu_time_ms = frame_timer_.elapsed_ms();
    
    // Record other frame metrics
    current_frame_.gpu_time_ms = gpu_time_ms;
    current_frame_.draw_calls = draw_calls;
    current_frame_.triangle_count = triangle_count;
    
    // Store in ring buffer
    frame_history_[history_write_index_] = current_frame_;
    
    // Advance ring buffer index
    history_write_index_ = (history_write_index_ + 1) % history_size_;
    
    // Update count (saturate at history_size_)
    if (history_count_ < history_size_) {
        history_count_++;
    }
}

void TelemetrySystem::begin_pass(const std::string& pass_name) {
    if (!enabled_) {
        return;  // Zero cost when disabled
    }

    current_pass_name_ = pass_name;
    pass_timer_.reset();
    pass_in_progress_ = true;
}

void TelemetrySystem::end_pass() {
    if (!enabled_ || !pass_in_progress_) {
        return;  // Zero cost when disabled or no pass in progress
    }

    double duration_ms = pass_timer_.elapsed_ms();
    current_frame_.pass_timings.emplace_back(current_pass_name_, duration_ms);
    
    pass_in_progress_ = false;
    current_pass_name_.clear();
}

const FrameTelemetryData* TelemetrySystem::get_frame(uint32_t frames_ago) const {
    if (!enabled_ || frames_ago >= history_count_) {
        return nullptr;
    }

    // Calculate ring buffer index
    // For frames_ago=0, we want the most recent completed frame
    // For frames_ago=1, we want the frame before that, etc.
    int32_t index = static_cast<int32_t>(history_write_index_) - 1 - static_cast<int32_t>(frames_ago);
    if (index < 0) {
        index += history_size_;
    }

    return &frame_history_[index];
}

uint32_t TelemetrySystem::get_pass_count() const {
    if (!enabled_) {
        return 0;
    }
    return static_cast<uint32_t>(current_frame_.pass_timings.size());
}

const PassTelemetryData* TelemetrySystem::get_pass_telemetry(uint32_t pass_index) const {
    if (!enabled_ || pass_index >= current_frame_.pass_timings.size()) {
        return nullptr;
    }
    return &current_frame_.pass_timings[pass_index];
}

void TelemetrySystem::clear_history() {
    history_write_index_ = 0;
    history_count_ = 0;
    
    // Clear all entries
    for (auto& frame : frame_history_) {
        frame = FrameTelemetryData();
    }
}

} // namespace astraeus
