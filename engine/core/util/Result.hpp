#ifndef ASTRAEUS_CORE_UTIL_RESULT_HPP
#define ASTRAEUS_CORE_UTIL_RESULT_HPP

/**
 * Result type for error handling without exceptions.
 */

#include <utility>
#include <type_traits>

namespace astraeus {

/**
 * Result<T> represents either a successful value T or an error code.
 * Similar to Rust's Result or std::expected (C++23).
 */
template <typename T>
class Result {
public:
    // Construct successful result
    static Result ok(T value) {
        return Result(std::move(value), true);
    }

    // Construct error result
    static Result error(int error_code = -1) {
        return Result(error_code);
    }

    // Check if result is successful
    bool is_ok() const noexcept {
        return is_ok_;
    }

    // Check if result is an error
    bool is_error() const noexcept {
        return !is_ok_;
    }

    // Get the value (only valid if is_ok())
    T& value() & {
        return value_;
    }

    const T& value() const & {
        return value_;
    }

    T&& value() && {
        return std::move(value_);
    }

    // Get error code (only valid if is_error())
    int error_code() const noexcept {
        return error_code_;
    }

    // Conversion to bool (true if ok)
    explicit operator bool() const noexcept {
        return is_ok_;
    }

private:
    Result(T value, bool)
        : value_(std::move(value))
        , error_code_(0)
        , is_ok_(true)
    {}

    Result(int error_code)
        : value_()
        , error_code_(error_code)
        , is_ok_(false)
    {}

    T value_;
    int error_code_;
    bool is_ok_;
};

// Specialization for void (no value, just success/error)
template <>
class Result<void> {
public:
    static Result ok() {
        return Result(true);
    }

    static Result error(int error_code = -1) {
        return Result(error_code);
    }

    bool is_ok() const noexcept {
        return is_ok_;
    }

    bool is_error() const noexcept {
        return !is_ok_;
    }

    int error_code() const noexcept {
        return error_code_;
    }

    explicit operator bool() const noexcept {
        return is_ok_;
    }

private:
    explicit Result(bool ok)
        : error_code_(0)
        , is_ok_(ok)
    {}

    explicit Result(int error_code)
        : error_code_(error_code)
        , is_ok_(false)
    {}

    int error_code_;
    bool is_ok_;
};

} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_RESULT_HPP
