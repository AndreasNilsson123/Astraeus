#ifndef ASTRAEUS_TIME_SYNC_HPP
#define ASTRAEUS_TIME_SYNC_HPP

#include <cstdint>
#include <atomic>

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

} // namespace astraeus

#endif // ASTRAEUS_TIME_SYNC_HPP
