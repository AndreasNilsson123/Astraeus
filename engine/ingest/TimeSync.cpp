#include "TimeSync.hpp"
#include <iostream>

namespace astraeus {

TimeSync::TimeSync()
    : is_initialized_(false)
    , sim_time_(0.0)
    , frame_number_(0)
    , last_update_time_(0.0)
    , target_rate_hz_(10.0) // Default 10 Hz
{
}

TimeSync::~TimeSync() {
    shutdown();
}

bool TimeSync::initialize() {
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

void TimeSync::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    is_initialized_ = false;
    std::cout << "[TimeSync] Shutdown" << std::endl;
}

void TimeSync::update_sim_time(double timestamp) {
    if (!is_initialized_) {
        return;
    }
    
    last_update_time_.store(sim_time_.load());
    sim_time_.store(timestamp);
}

double TimeSync::get_sim_time() const {
    return sim_time_.load();
}

void TimeSync::advance_frame() {
    if (!is_initialized_) {
        return;
    }
    
    frame_number_++;
}

uint64_t TimeSync::get_frame_number() const {
    return frame_number_.load();
}

void TimeSync::set_target_rate(double rate_hz) {
    target_rate_hz_.store(rate_hz);
}

double TimeSync::get_target_rate() const {
    return target_rate_hz_.load();
}

double TimeSync::get_delta_time() const {
    return sim_time_.load() - last_update_time_.load();
}

} // namespace astraeus
