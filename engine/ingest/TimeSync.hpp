#ifndef ASTRAEUS_TIME_SYNC_HPP
#define ASTRAEUS_TIME_SYNC_HPP

#include <cstdint>
#include <atomic>
#include <iostream>

namespace astraeus {

/**
 * TimeSync manages simulation time synchronization.
 * Tracks current simulation time, frame rate, and provides time utilities.
 */
class TimeSync {
public:
    TimeSync();
    ~TimeSync();
    
    /**
     * Initialize time sync.
     */
    bool initialize();
    
    /**
     * Shutdown.
     */
    void shutdown();
    
    /**
     * Update simulation time (called by ingest thread).
     */
    void update_sim_time(double timestamp);
    
    /**
     * Get current simulation time.
     */
    double get_sim_time() const;
    
    /**
     * Advance frame counter.
     */
    void advance_frame();
    
    /**
     * Get current frame number.
     */
    uint64_t get_frame_number() const;
    
    /**
     * Set target ingest rate (Hz).
     */
    void set_target_rate(double rate_hz);
    
    /**
     * Get target ingest rate (Hz).
     */
    double get_target_rate() const;
    
    /**
     * Get time delta since last update.
     */
    double get_delta_time() const;
    
private:
    bool is_initialized_;
    std::atomic<double> sim_time_;
    std::atomic<uint64_t> frame_number_;
    std::atomic<double> last_update_time_;
    std::atomic<double> target_rate_hz_;
};

// Inline implementations

inline TimeSync::TimeSync()
    : is_initialized_(false)
    , sim_time_(0.0)
    , frame_number_(0)
    , last_update_time_(0.0)
    , target_rate_hz_(10.0) // Default 10 Hz
{
}

inline TimeSync::~TimeSync() {
    shutdown();
}

inline bool TimeSync::initialize() {
    if (is_initialized_) {
        return true;
    }
    
    sim_time_.store(0.0);
    frame_number_.store(0);
    last_update_time_.store(0.0);
    
    is_initialized_ = true;
    std::cout << "[TimeSync] Initialized with target rate: " << target_rate_hz_.load() << " Hz" << std::endl;
    return true;
}

inline void TimeSync::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    is_initialized_ = false;
    std::cout << "[TimeSync] Shutdown" << std::endl;
}

inline void TimeSync::update_sim_time(double timestamp) {
    if (!is_initialized_) {
        return;
    }
    
    last_update_time_.store(sim_time_.load());
    sim_time_.store(timestamp);
}

inline double TimeSync::get_sim_time() const {
    return sim_time_.load();
}

inline void TimeSync::advance_frame() {
    if (!is_initialized_) {
        return;
    }
    
    frame_number_++;
}

inline uint64_t TimeSync::get_frame_number() const {
    return frame_number_.load();
}

inline void TimeSync::set_target_rate(double rate_hz) {
    target_rate_hz_.store(rate_hz);
}

inline double TimeSync::get_target_rate() const {
    return target_rate_hz_.load();
}

inline double TimeSync::get_delta_time() const {
    return sim_time_.load() - last_update_time_.load();
}

} // namespace astraeus

#endif // ASTRAEUS_TIME_SYNC_HPP
