# Fast Math Utilities Documentation

## Overview

The Fast Math utilities (`engine/core/util/FastMath.hpp`) provide high-performance approximate math functions optimized for visualization hot paths in the Astraeus engine. These functions trade some accuracy for speed and are designed for use cases where approximate results are acceptable.

## Quality Levels

Fast Math supports compile-time quality control via the `ASTRAEUS_FASTMATH_LEVEL` macro:

| Level | Description | Accuracy | Performance | Use Case |
|-------|-------------|----------|-------------|----------|
| 0 | Accurate fallback | Uses std:: functions | Standard performance | Testing, debugging, verification |
| 1 | Very fast | Lower accuracy (~0.1-1%) | Maximum speed | Non-critical visualizations, debug overlays |
| 2 | Balanced (default) | Good accuracy (~0.001-0.1%) | Good speed | General visualization, culling, LOD |
| 3 | Higher accuracy | Best accuracy (~0.0001-0.01%) | Moderate speed | Precision-sensitive operations |

To set the quality level, define the macro before including `FastMath.hpp` or in your build system:

```cpp
#define ASTRAEUS_FASTMATH_LEVEL 2
#include "core/util/FastMath.hpp"
```

Or in CMake:
```cmake
target_compile_definitions(your_target PRIVATE ASTRAEUS_FASTMATH_LEVEL=2)
```

## Intended Use Cases

Fast Math functions are designed for:

- **Camera controls and orbit calculations** - Computing angles, rotations, and transformations for camera movement
- **Culling heuristics** - Coarse rejection tests for frustum culling and visibility determination
- **LOD decisions** - Distance-based level-of-detail selection
- **Debug overlays** - Real-time visualization helpers like grid lines, axes, bounding boxes
- **Non-authoritative spatial queries** - Quick distance checks, approximate directions
- **Particle systems** - Position updates, velocity calculations
- **Animation interpolation** - Smooth transitions and easing functions

## Forbidden/High-Risk Use Cases

**DO NOT** use Fast Math for:

- **Serialization or network synchronization** - Different quality levels or platforms could produce inconsistent results
- **World-state truth** - Physics simulation, gameplay logic, or any authoritative state
- **Precision-critical picking** - Final picking results where accuracy matters (use for coarse tests only)
- **Financial calculations** - Any computation requiring exact precision
- **Cryptography or security** - Never use approximate math for security-sensitive operations
- **Cross-platform replay** - Determinism is only guaranteed within the same build configuration

## Available Functions

### Fast Inverse Square Root

```cpp
float fastInvSqrt(float x)  // Returns 1/sqrt(x)
float fastRSqrt(float x)    // Alias for fastInvSqrt
```

**Domain:** x > 0  
**Error Bounds:**
- Level 1: ~0.175%
- Level 2+: ~0.001%

**Edge Cases:**
- Returns +Inf for x = 0
- Returns NaN for x < 0 or x = NaN

**Example:**
```cpp
float inv_dist = fastInvSqrt(distance_squared);
```

### Fast Square Root

```cpp
float fastSqrt(float x)  // Returns sqrt(x)
```

**Domain:** x ≥ 0  
**Error Bounds:**
- Level 1: ~0.175%
- Level 2+: ~0.001%

**Edge Cases:**
- Returns 0 for x = 0
- Returns NaN for x < 0

**Example:**
```cpp
float distance = fastSqrt(dx*dx + dy*dy + dz*dz);
```

### Fast Trigonometric Functions

```cpp
float fastSin(float x)                    // Returns sin(x)
float fastCos(float x)                    // Returns cos(x)
void fastSinCos(float x, float& s, float& c)  // Computes both simultaneously
```

**Domain:** All real numbers (range reduction applied for |x| > π)  
**Tested Range:** [-π, π] for best accuracy; larger values use modulo reduction  
**Error Bounds:**
- Level 1: ~0.001 absolute error
- Level 2+: ~0.0001 absolute error

**Monotonicity:** Preserved in [-π/2, π/2] for sine

**Example:**
```cpp
// Camera orbit calculation
float s, c;
fastSinCos(angle, s, c);
camera_x = radius * c;
camera_z = radius * s;
```

### Fast Arctangent

```cpp
float fastAtan2(float y, float x)  // Returns atan2(y, x) in radians
```

**Domain:** All real (x, y) except (0, 0)  
**Range:** [-π, π]  
**Error Bounds:**
- Level 1: ~0.01 radians (~0.57°)
- Level 2+: ~0.005 radians (~0.29°)

**Edge Cases:**
- Returns 0 for (0, 0)
- Correctly handles all quadrants

**Example:**
```cpp
// Compute angle to target for camera look-at
float angle = fastAtan2(target_z - pos_z, target_x - pos_x);
```

### Fast Exponential Functions

```cpp
float fastExp(float x)   // Returns e^x
float fastExp2(float x)  // Returns 2^x
```

**Domain:** Practical range [-10, 10] for fastExp, [-30, 30] for fastExp2  
**Error Bounds:**
- Level 1: ~1%
- Level 2+: ~0.1%

**Edge Cases:** Values outside practical range are clamped to prevent overflow/underflow

**Example:**
```cpp
// Exponential fog calculation
float fog_factor = fastExp(-distance * fog_density);
```

### Vector Helpers

```cpp
float fastLength(float x, float y, float z)
void fastNormalize(float& x, float& y, float& z)
void fastNormalize(float x, float y, float z, float& out_x, float& out_y, float& out_z)
```

**Error Bounds:** Same as fastSqrt (~0.001-0.175% depending on quality level)

**Edge Cases:**
- fastNormalize returns (0, 0, 0) for zero-length vectors

**Example:**
```cpp
// Compute view direction
float dx = target_x - camera_x;
float dy = target_y - camera_y;
float dz = target_z - camera_z;
fastNormalize(dx, dy, dz);
```

### Utility Functions

```cpp
float fastRecip(float x)                              // Returns 1/x
bool isApproxZero(float x, float epsilon = 1e-6f)     // Check if ~0
bool isApproxEqual(float a, float b, float epsilon = 1e-6f)  // Check if a ≈ b
```

## Performance Characteristics

Typical speedups compared to std:: functions (measured on x86-64):

| Function | Level 1 | Level 2 | Level 3 |
|----------|---------|---------|---------|
| fastInvSqrt | 3-4x | 2-3x | 1.5-2x |
| fastSqrt | 2-3x | 1.5-2x | 1.2-1.5x |
| fastSin/fastCos | 3-5x | 2-3x | 1.5-2x |
| fastAtan2 | 4-6x | 3-4x | 2-3x |
| fastExp | 3-5x | 2-3x | 1.5-2x |
| fastNormalize | 2-3x | 1.5-2x | 1.2-1.5x |

**Note:** Actual performance depends on CPU architecture, compiler optimizations, and data access patterns.

## Usage Guidelines

### Opt-In Philosophy

Fast Math functions are **opt-in only**. You must explicitly call them; there is no global override of standard functions.

```cpp
// Explicit opt-in - GOOD
float result = astraeus::math::fast::fastSin(angle);

// Standard math is unchanged - GOOD
float precise_result = std::sin(angle);
```

### Best Practices

1. **Use for visualization, not truth**
   - Use Fast Math for display, rendering, and UI calculations
   - Use standard math for authoritative state and physics

2. **Profile before optimizing**
   - Measure actual performance impact before replacing std:: functions
   - Focus on hot paths identified by profiling

3. **Start with level 2**
   - Level 2 provides good balance of speed and accuracy for most cases
   - Use level 1 only if profiling shows it's necessary
   - Use level 3 if you need better accuracy but still want speedup

4. **Test error bounds**
   - Verify that the error bounds are acceptable for your use case
   - Add assertions or checks in debug builds if needed

5. **Document usage**
   - Comment why Fast Math is appropriate for each use case
   - Note if future code should maintain the approximation

### Example: Camera Orbit Implementation

```cpp
// Good use of Fast Math for camera control
void updateCameraOrbit(float angle_h, float angle_v, float radius) {
    // Use fast trig for real-time camera movement
    // Acceptable error: ~0.0001 won't be noticeable in camera position
    float sin_h, cos_h;
    astraeus::math::fast::fastSinCos(angle_h, sin_h, cos_h);
    
    float sin_v, cos_v;
    astraeus::math::fast::fastSinCos(angle_v, sin_v, cos_v);
    
    camera_pos_x = radius * cos_v * cos_h;
    camera_pos_y = radius * sin_v;
    camera_pos_z = radius * cos_v * sin_h;
    
    // Normalize look direction with fast normalize
    float dir_x = -camera_pos_x;
    float dir_y = -camera_pos_y;
    float dir_z = -camera_pos_z;
    astraeus::math::fast::fastNormalize(dir_x, dir_y, dir_z);
}
```

### Example: Frustum Culling

```cpp
// Good use of Fast Math for coarse culling
bool isInFrustum(float x, float y, float z, const Frustum& frustum) {
    // Fast distance check for coarse culling
    // If this passes, can do precise test later if needed
    float dist = astraeus::math::fast::fastLength(x, y, z);
    
    if (dist > frustum.far_plane) {
        return false;  // Definitely outside
    }
    
    // ... other culling tests ...
    return true;
}
```

## Testing

### Unit Tests

Run the unit tests to verify correctness:

```bash
./build/bin/fastmath_test
```

The tests verify:
- Error bounds for all quality levels
- Edge case handling (0, NaN, Inf)
- Consistency across input ranges
- Mathematical identities (e.g., sin²+cos²=1)

### Benchmarks

Run the benchmarks to measure performance:

```bash
./build/bin/fastmath_benchmark
```

The benchmarks compare Fast Math against std:: functions and report speedup factors.

## Implementation Details

### Algorithms Used

- **fastInvSqrt**: Quake III algorithm with optional Newton-Raphson refinement
- **fastSin/fastCos**: Polynomial approximation (3rd to 7th order depending on quality level)
- **fastAtan2**: Minimax polynomial approximation with quadrant handling
- **fastExp**: Polynomial approximation (level 1) or bit manipulation with 2^x decomposition (level 2+)

### Determinism

Fast Math functions are deterministic for a given:
- Quality level (ASTRAEUS_FASTMATH_LEVEL)
- Compiler and optimization flags
- Target platform

They are **NOT** deterministic across:
- Different quality levels
- Different platforms (x86 vs ARM)
- Different compilers (GCC vs Clang vs MSVC)

### Platform Support

Fast Math is tested on:
- Linux x86-64 (GCC, Clang)
- Windows x86-64 (MSVC, MinGW)

The library is header-only and uses standard C++17, so it should work on other platforms, but testing is recommended.

## Troubleshooting

### Unexpected Results

If Fast Math produces unexpected results:

1. **Check quality level**: Ensure ASTRAEUS_FASTMATH_LEVEL is set correctly
2. **Verify domain**: Check that inputs are within supported ranges
3. **Test with level 0**: Compare results against std:: functions
4. **Run unit tests**: Verify implementation on your platform

### Performance Not as Expected

If Fast Math is slower than expected:

1. **Enable optimizations**: Use -O3 or /O2 compiler flags
2. **Check inlining**: Ensure functions are being inlined (use inline in hot paths)
3. **Profile hotspots**: Use a profiler to identify actual bottlenecks
4. **Consider SIMD**: For batch operations, consider vectorized implementations

### Accuracy Issues

If accuracy is insufficient:

1. **Increase quality level**: Try level 3 for better accuracy
2. **Use standard math**: For critical calculations, use std:: functions
3. **Hybrid approach**: Use Fast Math for coarse tests, std:: for final results

## Future Enhancements

Potential future improvements:

- SIMD vectorized versions (SSE/AVX/NEON)
- Additional functions (log, log2, pow)
- Platform-specific optimizations
- Vectorized batch operations
- Integration with math libraries (GLM, Eigen)

## References

- [Fast Inverse Square Root (Wikipedia)](https://en.wikipedia.org/wiki/Fast_inverse_square_root)
- [Approximate Math Functions](https://www.iquilezles.org/www/articles/functions/functions.htm)
- [Performance of Sine and Cosine Approximations](https://stackoverflow.com/questions/18662261/fastest-implementation-of-sine-cosine-and-square-root-in-c-doesnt-need-to-b)
