// ============================================================================
// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY
// Generated from: abi_structs_schema.yaml
// Schema version: 1.0.0
// Generated at: 2026-01-30 17:39:43
// Schema hash: 019b094a389af9f3
// ============================================================================

#ifndef ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H
#define ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Schema version and generation metadata
#define ASTRAEUS_ABI_SCHEMA_VERSION "1.0.0"
#define ASTRAEUS_ABI_GENERATION_TIMESTAMP "2026-01-30 17:39:43"
#define ASTRAEUS_ABI_SCHEMA_HASH "019b094a389af9f3"

// Frame statistics (exposed to Java)
typedef struct {
    uint64_t frame_number;  // Current frame number
    double delta_time_ms;  // Time since last frame in milliseconds
    double render_time_ms;  // Render time for this frame in milliseconds
    uint32_t draw_calls;  // Number of draw calls this frame
    uint32_t triangle_count;  // Total triangles rendered this frame
    uint32_t entity_count;  // Number of active entities
    uint8_t _padding[4];  // Padding for 64-bit alignment
} FrameStats;

// Compile-time size and alignment checks for FrameStats
#ifdef __cplusplus
static_assert(sizeof(FrameStats) % 8 == 0,
              "FrameStats size must be aligned to 8 bytes");
#endif

// Telemetry frame statistics (for telemetry system)
typedef struct {
    uint64_t frame_number;  // Current frame number
    double cpu_time_ms;  // CPU time in milliseconds
    double gpu_time_ms;  // GPU time in milliseconds (placeholder)
    double total_time_ms;  // Total frame time in milliseconds
    uint32_t draw_calls;  // Number of draw calls
    uint32_t triangle_count;  // Total triangles rendered
    uint8_t pass_count;  // Number of active passes this frame
    uint8_t _padding[7];  // Explicit padding for 64-bit alignment
} TelemetryFrameStats;

// Compile-time size and alignment checks for TelemetryFrameStats
#ifdef __cplusplus
static_assert(sizeof(TelemetryFrameStats) % 8 == 0,
              "TelemetryFrameStats size must be aligned to 8 bytes");
#endif

// Viewport configuration
typedef struct {
    uint32_t width;  // Viewport width in pixels
    uint32_t height;  // Viewport height in pixels
    float aspect_ratio;  // Aspect ratio (width/height)
    uint8_t _padding[4];  // Padding for alignment
} ViewportConfig;

// Compile-time size and alignment checks for ViewportConfig
#ifdef __cplusplus
static_assert(sizeof(ViewportConfig) % 4 == 0,
              "ViewportConfig size must be aligned to 4 bytes");
#endif

// Multi-line description
typedef struct {
    void* data;  // Pointer to backing buffer (stable, never reallocated)
    uint32_t width;  // Current viewport width (may be <= max_backing_width)
    uint32_t height;  // Current viewport height (may be <= max_backing_height)
    uint32_t stride;  // Row stride in bytes
    uint32_t format;  // PixelFormat enum value
    uint32_t max_backing_width;  // Maximum width of backing buffer
    uint32_t max_backing_height;  // Maximum height of backing buffer
    uint32_t max_backing_size;  // Total size of backing buffer in bytes
} PixelBufferView;

// Compile-time size and alignment checks for PixelBufferView
#ifdef __cplusplus
static_assert(sizeof(PixelBufferView) % 8 == 0,
              "PixelBufferView size must be aligned to 8 bytes");
#endif

// Configuration for readback buffers
typedef struct {
    uint32_t max_width;  // Maximum expected viewport width
    uint32_t max_height;  // Maximum expected viewport height
    uint32_t format;  // PixelFormat enum value
    bool enable_double_buffer;  // Enable double-buffered readback (safer, slightly slower)
} ReadbackConfig;

// Compile-time size and alignment checks for ReadbackConfig
#ifdef __cplusplus
static_assert(sizeof(ReadbackConfig) % 4 == 0,
              "ReadbackConfig size must be aligned to 4 bytes");
#endif

// Picking result
typedef struct {
    uint32_t entity_id;  // ID of picked entity (0 if no hit)
    float depth;  // Depth value at pick location
    float world_x;  // World space X coordinate
    float world_y;  // World space Y coordinate
    float world_z;  // World space Z coordinate
    bool hit;  // Whether a valid entity was hit
    uint8_t _padding[3];  // Padding for alignment
} PickResult;

// Compile-time size and alignment checks for PickResult
#ifdef __cplusplus
static_assert(sizeof(PickResult) % 4 == 0,
              "PickResult size must be aligned to 4 bytes");
#endif

// Engine configuration
typedef struct {
    uint32_t initial_width;  // Initial viewport width
    uint32_t initial_height;  // Initial viewport height
    bool enable_validation;  // Enable validation layers
    bool enable_debug_output;  // Enable debug output
    void* log_file_path;  // Path to log file (const char*, can be NULL)
} EngineConfig;

// Compile-time size and alignment checks for EngineConfig
#ifdef __cplusplus
static_assert(sizeof(EngineConfig) % 8 == 0,
              "EngineConfig size must be aligned to 8 bytes");
#endif

#ifdef __cplusplus
}
#endif

#endif // ASTRAEUS_ENGINE_ABI_STRUCTS_GEN_H
