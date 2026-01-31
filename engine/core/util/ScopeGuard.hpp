#ifndef ASTRAEUS_CORE_UTIL_SCOPE_GUARD_HPP
#define ASTRAEUS_CORE_UTIL_SCOPE_GUARD_HPP

/**
 * RAII scope guard for cleanup actions.
 */

#include <utility>
#include <type_traits>

namespace astraeus {

/**
 * ScopeGuard executes a callable on destruction.
 * Useful for cleanup that must happen regardless of how scope is exited.
 */
template <typename Func>
class ScopeGuard {
public:
    explicit ScopeGuard(Func&& func)
        : func_(std::forward<Func>(func))
        , active_(true)
    {}

    ~ScopeGuard() {
        if (active_) {
            func_();
        }
    }

    // Non-copyable
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    // Movable
    ScopeGuard(ScopeGuard&& other) noexcept
        : func_(std::move(other.func_))
        , active_(other.active_)
    {
        other.active_ = false;
    }

    // Dismiss the guard (prevent execution)
    void dismiss() noexcept {
        active_ = false;
    }

private:
    Func func_;
    bool active_;
};

// Helper to create a scope guard with type deduction
template <typename Func>
inline ScopeGuard<Func> make_scope_guard(Func&& func) {
    return ScopeGuard<Func>(std::forward<Func>(func));
}

// Macro for convenience
#define ASTRAEUS_SCOPE_EXIT(code) \
    auto ASTRAEUS_CONCAT(scope_guard_, __LINE__) = ::astraeus::make_scope_guard([&]() { code; })

// Helper macro for concatenation
#define ASTRAEUS_CONCAT_IMPL(a, b) a##b
#define ASTRAEUS_CONCAT(a, b) ASTRAEUS_CONCAT_IMPL(a, b)

} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_SCOPE_GUARD_HPP
