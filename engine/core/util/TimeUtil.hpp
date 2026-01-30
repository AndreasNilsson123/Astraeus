#ifndef ASTRAEUS_CORE_UTIL_TIME_UTIL_HPP
#define ASTRAEUS_CORE_UTIL_TIME_UTIL_HPP

/**
 * Time utility functions.
 */

#include <chrono>
#include <cstdint>

namespace astraeus {
namespace time_util {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = Clock::duration;

/**
 * Get current high-resolution time point.
 */
inline TimePoint now() {
    return Clock::now();
}

/**
 * Convert duration to milliseconds.
 */
inline double to_milliseconds(Duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
}

/**
 * Convert duration to seconds.
 */
inline double to_seconds(Duration duration) {
    return std::chrono::duration<double>(duration).count();
}

/**
 * Convert duration to microseconds.
 */
inline int64_t to_microseconds(Duration duration) {
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

/**
 * Simple timer for measuring elapsed time.
 */
class Timer {
public:
    Timer() : start_(now()) {}

    void reset() {
        start_ = now();
    }

    Duration elapsed() const {
        return now() - start_;
    }

    double elapsed_ms() const {
        return to_milliseconds(elapsed());
    }

    double elapsed_seconds() const {
        return to_seconds(elapsed());
    }

private:
    TimePoint start_;
};

} // namespace time_util
} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_TIME_UTIL_HPP
