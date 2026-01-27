#ifndef ASTRAEUS_ENGINE_API_H
#define ASTRAEUS_ENGINE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// VERSION AND BUILD INFO
// =============================================================================

#define ASTRAEUS_VERSION_MAJOR 0
#define ASTRAEUS_VERSION_MINOR 1
#define ASTRAEUS_VERSION_PATCH 0

// ABI Compatibility Notes:
// - PixelBufferView uses fixed-size backing memory (max_backing_size)
// - Resizing only updates viewport region, never reallocates backing memory
// - This ensures JavaFX PixelBuffer memory stability and prevents crashes

// =============================================================================
// OPAQUE HANDLES
// =============================================================================

typedef struct AstraeusEngine* EngineHandle;

// =============================================================================
// POD STRUCTS FOR FFM
// =============================================================================

// Frame statistics (exposed to Java)
typedef struct {
    uint64_t frame_number;
    double delta_time_ms;
    double render_time_ms;
    uint32_t draw_calls;
    uint32_t triangle_count;
    uint32_t entity_count;
} FrameStats;

// Viewport configuration
typedef struct {
    uint32_t width;
    uint32_t height;
    float aspect_ratio;
} ViewportConfig;

// Pixel format enumeration
typedef enum {
    PIXEL_FORMAT_RGBA8 = 0,    // Standard RGBA, 8-bit per channel
    PIXEL_FORMAT_BGRA8 = 1,    // BGRA format (common for Windows/JavaFX)
    PIXEL_FORMAT_ARGB8 = 2,    // ARGB format
    PIXEL_FORMAT_R32UI = 3     // 32-bit unsigned int (for ID buffer)
} PixelFormat;

// Pixel buffer view for readback (zero-copy, fixed backing size)
// IMPORTANT: The backing memory (data pointer) is allocated once with max_backing_size
// and remains stable for the engine's lifetime. Only the viewport region (width, height)
// changes on resize. This ensures JavaFX PixelBuffer memory stability.
typedef struct {
    void* data;                    // Pointer to backing buffer (stable, never reallocated)
    uint32_t width;                // Current viewport width (may be <= max_backing_width)
    uint32_t height;               // Current viewport height (may be <= max_backing_height)
    uint32_t stride;               // Row stride in bytes
    uint32_t format;               // PixelFormat enum value
    uint32_t max_backing_width;    // Maximum width of backing buffer
    uint32_t max_backing_height;   // Maximum height of backing buffer
    uint32_t max_backing_size;     // Total size of backing buffer in bytes
} PixelBufferView;

// Configuration for readback buffers
typedef struct {
    uint32_t max_width;            // Maximum expected viewport width
    uint32_t max_height;           // Maximum expected viewport height
    uint32_t format;               // PixelFormat enum value
    bool enable_double_buffer;     // Enable double-buffered readback (safer, slightly slower)
} ReadbackConfig;

// Picking result
typedef struct {
    uint32_t entity_id;
    float depth;
    float world_x;
    float world_y;
    float world_z;
    bool hit;
} PickResult;

// Engine configuration
typedef struct {
    uint32_t initial_width;
    uint32_t initial_height;
    bool enable_validation;
    bool enable_debug_output;
    const char* log_file_path;
} EngineConfig;

// =============================================================================
// ENGINE LIFECYCLE
// =============================================================================

/**
 * Create a new engine instance.
 * @param config Engine configuration
 * @return Opaque engine handle or NULL on failure
 */
EngineHandle astraeus_create_engine(const EngineConfig* config);

/**
 * Destroy the engine instance and free all resources.
 * @param engine Engine handle
 */
void astraeus_destroy_engine(EngineHandle engine);

/**
 * Check if the engine is valid and initialized.
 * @param engine Engine handle
 * @return true if valid, false otherwise
 */
bool astraeus_is_valid(EngineHandle engine);

// =============================================================================
// RENDERING
// =============================================================================

/**
 * Begin a new frame.
 * @param engine Engine handle
 * @param delta_time Time since last frame in seconds
 */
void astraeus_begin_frame(EngineHandle engine, double delta_time);

/**
 * End the current frame and present.
 * @param engine Engine handle
 */
void astraeus_end_frame(EngineHandle engine);

/**
 * Resize the viewport.
 * IMPORTANT: This only changes the viewport region, NOT the backing buffer size.
 * The backing buffer is allocated once at creation with max size and remains stable.
 * This ensures JavaFX PixelBuffer memory stability.
 * @param engine Engine handle
 * @param width New viewport width (must be <= max_backing_width)
 * @param height New viewport height (must be <= max_backing_height)
 */
void astraeus_resize_viewport(EngineHandle engine, uint32_t width, uint32_t height);

/**
 * Configure readback buffers (color and ID buffers).
 * Must be called before first frame to set fixed backing buffer size.
 * @param engine Engine handle
 * @param color_config Configuration for color buffer (can be NULL to use defaults)
 * @param id_config Configuration for ID buffer (can be NULL to use defaults)
 * @return true on success, false on failure
 */
bool astraeus_configure_readback(EngineHandle engine, 
                                  const ReadbackConfig* color_config,
                                  const ReadbackConfig* id_config);

/**
 * Get the current frame statistics.
 * @param engine Engine handle
 * @param out_stats Output frame stats
 */
void astraeus_get_frame_stats(EngineHandle engine, FrameStats* out_stats);

/**
 * Get a view of the color buffer for readback (zero-copy).
 * Buffer is valid until next frame.
 * @param engine Engine handle
 * @return Pixel buffer view
 */
PixelBufferView astraeus_get_color_buffer(EngineHandle engine);

/**
 * Get a view of the ID buffer for picking (zero-copy).
 * Buffer is valid until next frame.
 * @param engine Engine handle
 * @return Pixel buffer view
 */
PixelBufferView astraeus_get_id_buffer(EngineHandle engine);

// =============================================================================
// SCENE MANAGEMENT
// =============================================================================

/**
 * Create a new entity in the scene.
 * @param engine Engine handle
 * @return Entity ID (handle-based)
 */
uint32_t astraeus_create_entity(EngineHandle engine);

/**
 * Destroy an entity.
 * @param engine Engine handle
 * @param entity_id Entity ID
 */
void astraeus_destroy_entity(EngineHandle engine, uint32_t entity_id);

/**
 * Set entity transform.
 * @param engine Engine handle
 * @param entity_id Entity ID
 * @param pos_x Position X
 * @param pos_y Position Y
 * @param pos_z Position Z
 * @param rot_x Rotation X (radians)
 * @param rot_y Rotation Y (radians)
 * @param rot_z Rotation Z (radians)
 * @param scale_x Scale X
 * @param scale_y Scale Y
 * @param scale_z Scale Z
 */
void astraeus_set_entity_transform(EngineHandle engine, uint32_t entity_id,
                                   float pos_x, float pos_y, float pos_z,
                                   float rot_x, float rot_y, float rot_z,
                                   float scale_x, float scale_y, float scale_z);

// =============================================================================
// PICKING
// =============================================================================

/**
 * Perform picking at screen coordinates.
 * @param engine Engine handle
 * @param screen_x Screen X coordinate
 * @param screen_y Screen Y coordinate
 * @return Pick result
 */
PickResult astraeus_pick(EngineHandle engine, uint32_t screen_x, uint32_t screen_y);

// =============================================================================
// DATA INGESTION
// =============================================================================

/**
 * Ingest external simulation data snapshot.
 * @param engine Engine handle
 * @param data Pointer to simulation data
 * @param size Size of data in bytes
 * @param format Data format identifier
 * @return true on success, false on failure
 */
bool astraeus_ingest_data(EngineHandle engine, const void* data, uint32_t size, uint32_t format);

// =============================================================================
// CAMERA CONTROL
// =============================================================================

/**
 * Set camera position and target.
 * @param engine Engine handle
 * @param eye_x Camera position X
 * @param eye_y Camera position Y
 * @param eye_z Camera position Z
 * @param target_x Look-at target X
 * @param target_y Look-at target Y
 * @param target_z Look-at target Z
 * @param up_x Up vector X
 * @param up_y Up vector Y
 * @param up_z Up vector Z
 */
void astraeus_set_camera(EngineHandle engine,
                        float eye_x, float eye_y, float eye_z,
                        float target_x, float target_y, float target_z,
                        float up_x, float up_y, float up_z);

/**
 * Set camera projection parameters.
 * @param engine Engine handle
 * @param fov_degrees Field of view in degrees
 * @param near_plane Near clipping plane
 * @param far_plane Far clipping plane
 */
void astraeus_set_camera_projection(EngineHandle engine, float fov_degrees, float near_plane, float far_plane);

#ifdef __cplusplus
}
#endif

#endif // ASTRAEUS_ENGINE_API_H
