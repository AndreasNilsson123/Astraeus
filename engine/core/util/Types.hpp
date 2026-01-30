#ifndef ASTRAEUS_CORE_UTIL_TYPES_HPP
#define ASTRAEUS_CORE_UTIL_TYPES_HPP

/**
 * Common type definitions for the Astraeus engine.
 */

#include <cstdint>
#include <cstddef>

namespace astraeus {

// Unsigned integer types
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Signed integer types
using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

// Floating point types
using f32 = float;
using f64 = double;

// Size types
using usize = std::size_t;
using isize = std::ptrdiff_t;

// Byte type
using byte = std::uint8_t;

} // namespace astraeus

#endif // ASTRAEUS_CORE_UTIL_TYPES_HPP
