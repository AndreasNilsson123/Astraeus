#ifndef ASTRAEUS_CORE_UTIL_ASSERT_HPP
#define ASTRAEUS_CORE_UTIL_ASSERT_HPP

/**
 * Assertion utilities for the Astraeus engine.
 */

#include <cassert>
#include <iostream>

namespace astraeus {

// Use standard assert for now, can be extended with custom behavior
#define ASTRAEUS_ASSERT(condition) assert(condition)

// Assert with message
#define ASTRAEUS_ASSERT_MSG(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "[Astraeus Assert] " << message << std::endl; \
            assert(condition); \
        } \
    } while (0)

// Verify (always evaluated, even in release builds)
#define ASTRAEUS_VERIFY(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "[Astraeus Verify Failed] " << #condition << std::endl; \
        } \
    } while (0)

} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_ASSERT_HPP
