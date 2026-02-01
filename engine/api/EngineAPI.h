#ifndef ASTRAEUS_ENGINE_API_H
#define ASTRAEUS_ENGINE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Include generated ABI structs
#include "generated/EngineABI_Structs.h"

#if defined(_WIN32)
    // 3 modes:
    //  - ASTRAEUS_API_STATIC: building/using as static/monolithic (no import/export)
    //  - ASTRAEUS_BUILDING_DLL: building the DLL (dllexport)
    //  - default: consuming the DLL (dllimport)
#if defined(ASTRAEUS_API_STATIC)
    #define ASTRAEUS_API
    #elif defined(ASTRAEUS_BUILDING_DLL)
        #define ASTRAEUS_API __declspec(dllexport)
    #else
        #define ASTRAEUS_API __declspec(dllimport)
    #endif
#else
    // If you later want ELF visibility:
    // #define ASTRAEUS_API __attribute__((visibility("default")))
    #define ASTRAEUS_API
#endif



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
typedef struct AstraeusViewport* ViewportHandle;
typedef struct AstraeusCamera* CameraHandle;
typedef struct AstraeusMaterial* MaterialHandle;

// =============================================================================
// RESULT CODES
// =============================================================================

typedef enum {
    ASTRAEUS_SUCCESS = 0,
    ASTRAEUS_ERROR_INVALID_HANDLE = 1,
    ASTRAEUS_ERROR_INVALID_PARAMETER = 2,
    ASTRAEUS_ERROR_OUT_OF_MEMORY = 3,
    ASTRAEUS_ERROR_NOT_INITIALIZED = 4,
    ASTRAEUS_ERROR_UNKNOWN = 255
} AstraeusResult;

// =============================================================================
// POD STRUCTS FOR FFM
// =============================================================================
// NOTE: ABI POD structs are now defined in EngineABI_Structs.gen.h (auto-generated)
// This includes: FrameStats, TelemetryFrameStats, ViewportConfig, PixelBufferView,
// ReadbackConfig, PickResult, EngineConfig, CameraDesc, MaterialDesc

// Pixel format enumeration
typedef enum {
    PIXEL_FORMAT_RGBA8 = 0,    // Standard RGBA, 8-bit per channel
    PIXEL_FORMAT_BGRA8 = 1,    // BGRA format (common for Windows/JavaFX)
    PIXEL_FORMAT_ARGB8 = 2,    // ARGB format
    PIXEL_FORMAT_R32UI = 3     // 32-bit unsigned int (for ID buffer)
} PixelFormat;

// Camera mode enumeration
typedef enum {
    CAMERA_MODE_ORBIT = 0,
    CAMERA_MODE_FLY = 1,
    CAMERA_MODE_PAN = 2
} CameraMode;

// Alpha mode enumeration
typedef enum {
    ALPHA_MODE_OPAQUE = 0,
    ALPHA_MODE_BLEND = 1,
    ALPHA_MODE_MASK = 2
} AlphaMode;

// =============================================================================
// ENGINE LIFECYCLE
// =============================================================================

/**
 * Get the API version number.
 * Version format: (MAJOR << 16) | (MINOR << 8) | PATCH
 * @return API version number
 */
ASTRAEUS_API uint32_t astraeus_api_version(void);

/**
 * Get the last error message for the engine.
 * @param engine Engine handle
 * @return Error message string (valid until next call, or NULL if no error)
 */
ASTRAEUS_API const char* astraeus_last_error(EngineHandle engine);

/**
 * Create a new engine instance.
 * @param config Engine configuration
 * @return Opaque engine handle or NULL on failure
 */
ASTRAEUS_API EngineHandle astraeus_create_engine(const EngineConfig* config);

/**
 * Destroy the engine instance and free all resources.
 * @param engine Engine handle
 */
ASTRAEUS_API void astraeus_destroy_engine(EngineHandle engine);

/**
 * Check if the engine is valid and initialized.
 * @param engine Engine handle
 * @return true if valid, false otherwise
 */
ASTRAEUS_API bool astraeus_is_valid(EngineHandle engine);

// =============================================================================
// RENDERING
// =============================================================================

/**
 * Begin a new frame.
 * @param engine Engine handle
 * @param delta_time Time since last frame in seconds
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_begin_frame(EngineHandle engine, double delta_time);

/**
 * End the current frame and present.
 * @param engine Engine handle
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_end_frame(EngineHandle engine);

/**
 * Resize the viewport.
 * IMPORTANT: This only changes the viewport region, NOT the backing buffer size.
 * The backing buffer is allocated once at creation with max size and remains stable.
 * This ensures JavaFX PixelBuffer memory stability.
 * @param engine Engine handle
 * @param width New viewport width (must be <= max_backing_width)
 * @param height New viewport height (must be <= max_backing_height)
 */
ASTRAEUS_API void astraeus_resize_viewport(EngineHandle engine, uint32_t width, uint32_t height);

/**
 * Configure readback buffers (color and ID buffers).
 * Must be called before first frame to set fixed backing buffer size.
 * @param engine Engine handle
 * @param color_config Configuration for color buffer (can be NULL to use defaults)
 * @param id_config Configuration for ID buffer (can be NULL to use defaults)
 * @return true on success, false on failure
 */
ASTRAEUS_API bool astraeus_configure_readback(EngineHandle engine,
                                  const ReadbackConfig* color_config,
                                  const ReadbackConfig* id_config);

/**
 * Get the current frame statistics.
 * @param engine Engine handle
 * @param out_stats Output frame stats
 */
ASTRAEUS_API void astraeus_get_frame_stats(EngineHandle engine, FrameStats* out_stats);

/**
 * Get a view of the color buffer for readback (zero-copy).
 * Buffer is valid until next frame.
 * @param engine Engine handle
 * @param out_view Output pixel buffer view (must not be NULL)
 */
ASTRAEUS_API void astraeus_get_color_buffer(EngineHandle engine, PixelBufferView* out_view);

/**
 * Get a view of the ID buffer for picking (zero-copy).
 * Buffer is valid until next frame.
 * @param engine Engine handle
 * @param out_view Output pixel buffer view (must not be NULL)
 */
ASTRAEUS_API void astraeus_get_id_buffer(EngineHandle engine, PixelBufferView* out_view);

// =============================================================================
// VIEWPORT API (Render Session)
// =============================================================================

/**
 * Create a new viewport for rendering.
 * For MVP, creates a 1:1 mapping with the engine context.
 * @param engine Engine handle
 * @param config Viewport configuration
 * @param out_viewport Output viewport handle (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_viewport_create(EngineHandle engine, 
                                        const ViewportConfig* config, 
                                        ViewportHandle* out_viewport);

/**
 * Destroy a viewport and free its resources.
 * @param viewport Viewport handle
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_viewport_destroy(ViewportHandle viewport);

/**
 * Resize the viewport.
 * @param viewport Viewport handle
 * @param width New viewport width
 * @param height New viewport height
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_viewport_resize(ViewportHandle viewport, 
                                        uint32_t width, uint32_t height);

/**
 * Get the color buffer from the viewport.
 * @param viewport Viewport handle
 * @param out_view Output pixel buffer view (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_viewport_get_color(ViewportHandle viewport, 
                                           PixelBufferView* out_view);

/**
 * Get the ID buffer from the viewport for picking.
 * @param viewport Viewport handle
 * @param out_view Output pixel buffer view (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_viewport_get_idbuffer(ViewportHandle viewport, 
                                              PixelBufferView* out_view);

// =============================================================================
// CAMERA API (Render Session)
// =============================================================================

/**
 * Get the active camera for a viewport.
 * @param viewport Viewport handle
 * @param out_camera Output camera handle (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_camera_get_active(ViewportHandle viewport, 
                                          CameraHandle* out_camera);

/**
 * Get camera descriptor (read camera state).
 * @param camera Camera handle
 * @param out_desc Output camera descriptor (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_camera_get_desc(CameraHandle camera, 
                                        CameraDesc* out_desc);

/**
 * Set camera descriptor (update camera state).
 * @param camera Camera handle
 * @param desc Camera descriptor (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_camera_set_desc(CameraHandle camera, 
                                        const CameraDesc* desc);

/**
 * Destroy a camera handle (lightweight, does NOT destroy the actual camera).
 * @param camera Camera handle
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_camera_destroy(CameraHandle camera);

// =============================================================================
// MATERIALS API (MVP - Render Session)
// =============================================================================

/**
 * Create a new material.
 * @param engine Engine handle
 * @param desc Material descriptor (must not be NULL)
 * @param out_material Output material handle (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_material_create(EngineHandle engine, 
                                        const MaterialDesc* desc, 
                                        MaterialHandle* out_material);

/**
 * Update an existing material.
 * @param material Material handle
 * @param desc Material descriptor (must not be NULL)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_material_update(MaterialHandle material, 
                                        const MaterialDesc* desc);

/**
 * Destroy a material and free its resources.
 * @param material Material handle
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_material_destroy(MaterialHandle material);

/**
 * Assign a material to an entity.
 * @param engine Engine handle
 * @param entity_id Entity ID
 * @param material Material handle
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_entity_set_material(EngineHandle engine, 
                                            uint32_t entity_id, 
                                            MaterialHandle material);

// =============================================================================
// SCENE MANAGEMENT
// =============================================================================

/**
 * Create a new entity in the scene.
 * @param engine Engine handle
 * @return Entity ID (handle-based)
 */
ASTRAEUS_API uint32_t astraeus_create_entity(EngineHandle engine);

/**
 * Destroy an entity.
 * @param engine Engine handle
 * @param entity_id Entity ID
 */
ASTRAEUS_API void astraeus_destroy_entity(EngineHandle engine, uint32_t entity_id);

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
ASTRAEUS_API void astraeus_set_entity_transform(EngineHandle engine, uint32_t entity_id,
                                   float pos_x, float pos_y, float pos_z,
                                   float rot_x, float rot_y, float rot_z,
                                   float scale_x, float scale_y, float scale_z);

/**
 * Set entity renderable (visibility) state.
 * @param engine Engine handle
 * @param entity_id Entity ID
 * @param visible Whether entity should be rendered
 */
ASTRAEUS_API void astraeus_set_entity_renderable(EngineHandle engine, uint32_t entity_id, bool visible);

/**
 * Set entity color.
 * @param engine Engine handle
 * @param entity_id Entity ID
 * @param r Red component [0-1]
 * @param g Green component [0-1]
 * @param b Blue component [0-1]
 * @param a Alpha component [0-1]
 */
ASTRAEUS_API void astraeus_set_entity_color(EngineHandle engine, uint32_t entity_id,
                               float r, float g, float b, float a);

/**
 * Set entity trail (enable trail rendering with specified max length).
 * @param engine Engine handle
 * @param entity_id Entity ID
 * @param max_points Maximum number of trail points
 */
ASTRAEUS_API void astraeus_set_entity_trail(EngineHandle engine, uint32_t entity_id, uint32_t max_points);

/**
 * Apply entity snapshot at time t (WorldSync entry point).
 * Updates entity position and trail history.
 * @param engine Engine handle
 * @param entity_id Entity ID
 * @param pos_x Position X
 * @param pos_y Position Y
 * @param pos_z Position Z
 */
ASTRAEUS_API void astraeus_apply_entity_snapshot(EngineHandle engine, uint32_t entity_id,
                                    float pos_x, float pos_y, float pos_z);

// =============================================================================
// PICKING
// =============================================================================

/**
 * Perform picking at screen coordinates.
 * @param engine Engine handle
 * @param screen_x Screen X coordinate
 * @param screen_y Screen Y coordinate
 * @param pick_result out buffer
 * @return Pick result
 */
ASTRAEUS_API void astraeus_pick(EngineHandle engine, uint32_t screen_x, uint32_t screen_y, PickResult* pick_result);

// =============================================================================
// DATA INGESTION
// =============================================================================

/**
 * Data format identifiers for ingestion.
 */
typedef enum {
    ASTRAEUS_FORMAT_FIXED_BINARY = 0,  // Fixed binary format (default)
    ASTRAEUS_FORMAT_JSON = 1,          // JSON format (future)
    ASTRAEUS_FORMAT_CUSTOM = 255       // Custom format
} AstraeusDataFormat;

/**
 * Ingest status structure for polling job progress.
 * This is a POD struct safe for FFM boundary crossing.
 */
typedef struct {
    uint64_t job_id;           // Job identifier
    uint32_t format;           // Data format used
    uint32_t total_bytes;      // Total bytes to process
    uint32_t processed_bytes;  // Bytes processed so far
    uint8_t is_complete;       // true if job is complete
    uint8_t has_error;         // true if job had an error
    uint8_t _padding[2];       // Alignment padding
    char last_error[256];      // Last error message (if any)
} IngestStatus;

/**
 * Ingest external simulation data snapshot.
 * This function is thread-safe and returns immediately with a job ID.
 * Use astraeus_get_ingest_status() to poll for completion.
 * 
 * @param engine Engine handle
 * @param data Pointer to simulation data (must not be NULL)
 * @param size Size of data in bytes (must be > 0)
 * @param format Data format identifier (see AstraeusDataFormat)
 * @return Job ID (non-zero on success, 0 on failure)
 * 
 * Thread Safety: Safe to call from any thread
 * Lifetime: Data is copied internally; caller may free after return
 * 
 * Supported Formats:
 * - ASTRAEUS_FORMAT_FIXED_BINARY (0): Deterministic binary format with header
 * - ASTRAEUS_FORMAT_JSON (1): JSON format (not yet implemented)
 * - ASTRAEUS_FORMAT_CUSTOM (255): Custom format (requires registered decoder)
 */
ASTRAEUS_API uint64_t astraeus_ingest_data(EngineHandle engine, const void* data, uint32_t size, uint32_t format);

/**
 * Get the status of an ingest job.
 * This is a polling-based interface (no callbacks) for FFM safety.
 * 
 * @param engine Engine handle
 * @param job_id Job identifier returned from astraeus_ingest_data()
 * @param out_status Output status structure (must not be NULL)
 * @return true if job exists, false if job not found
 * 
 * Thread Safety: Safe to call from any thread
 */
ASTRAEUS_API bool astraeus_get_ingest_status(EngineHandle engine, uint64_t job_id, IngestStatus* out_status);

/**
 * Get the current simulation time from the ingest subsystem.
 * 
 * @param engine Engine handle
 * @return Current simulation time in seconds (0.0 if not initialized)
 */
ASTRAEUS_API double astraeus_get_sim_time(EngineHandle engine);

/**
 * Get the total number of snapshots ingested.
 * 
 * @param engine Engine handle
 * @return Total snapshot count
 */
ASTRAEUS_API uint64_t astraeus_get_snapshot_count(EngineHandle engine);

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
ASTRAEUS_API void astraeus_set_camera(EngineHandle engine,
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
ASTRAEUS_API void astraeus_set_camera_projection(EngineHandle engine, float fov_degrees, float near_plane, float far_plane);

// =============================================================================
// TELEMETRY
// =============================================================================

/**
 * Enable or disable telemetry collection.
 * When disabled, all telemetry operations have ZERO overhead.
 * @param engine Engine handle
 * @param enabled Whether to enable telemetry
 */
ASTRAEUS_API void astraeus_enable_telemetry(EngineHandle engine, bool enabled);

/**
 * Check if telemetry is enabled.
 * @param engine Engine handle
 * @return true if telemetry is enabled, false otherwise
 */
ASTRAEUS_API bool astraeus_is_telemetry_enabled(EngineHandle engine);

/**
 * Get telemetry frame statistics for the current frame.
 * @param engine Engine handle
 * @param out_stats Output telemetry frame stats (must not be NULL)
 */
ASTRAEUS_API void astraeus_get_telemetry_frame_stats(EngineHandle engine, TelemetryFrameStats* out_stats);

/**
 * Get telemetry history (ring buffer).
 * @param engine Engine handle
 * @param out_buffer Output buffer for frame stats (must not be NULL)
 * @param max_frames Maximum number of frames to retrieve
 * @return Number of frames actually written
 */
ASTRAEUS_API uint32_t astraeus_get_telemetry_history(EngineHandle engine, TelemetryFrameStats* out_buffer, uint32_t max_frames);

/**
 * Get the number of render passes in the current frame.
 * @param engine Engine handle
 * @return Number of render passes
 */
ASTRAEUS_API uint32_t astraeus_get_pass_count(EngineHandle engine);

/**
 * Get timing information for a specific render pass.
 * @param engine Engine handle
 * @param pass_index Index of the pass (0 to pass_count-1)
 * @param out_name_buffer Output buffer for pass name (must not be NULL)
 * @param name_buffer_size Size of the name buffer in bytes
 * @param out_time_ms Output pass timing in milliseconds (must not be NULL)
 * @return true if pass exists and data was retrieved, false otherwise
 */
ASTRAEUS_API bool astraeus_get_pass_timing(EngineHandle engine, uint32_t pass_index, 
                               char* out_name_buffer, uint32_t name_buffer_size, 
                               double* out_time_ms);

// =============================================================================
// COMMAND BUFFER API
// =============================================================================

/**
 * Submit a create entity command.
 * Returns immediately; command executed on next tick.
 * @param engine Engine handle
 * @param out_entity_id Pointer to receive new entity ID (can be NULL)
 */
ASTRAEUS_API void astraeus_command_create_entity(EngineHandle engine, uint32_t* out_entity_id);

/**
 * Submit a destroy entity command.
 * @param engine Engine handle
 * @param entity_id Entity to destroy
 */
ASTRAEUS_API void astraeus_command_destroy_entity(EngineHandle engine, uint32_t entity_id);

/**
 * Submit a set transform command.
 * @param engine Engine handle
 * @param entity_id Target entity
 * @param pos_x, pos_y, pos_z Position
 * @param rot_x, rot_y, rot_z Rotation (Euler angles)
 * @param scale_x, scale_y, scale_z Scale
 */
ASTRAEUS_API void astraeus_command_set_transform(EngineHandle engine, uint32_t entity_id,
                                    float pos_x, float pos_y, float pos_z,
                                    float rot_x, float rot_y, float rot_z,
                                    float scale_x, float scale_y, float scale_z);

/**
 * Submit an assign mesh command.
 * @param engine Engine handle
 * @param entity_id Target entity
 * @param mesh_id Mesh to assign
 */
ASTRAEUS_API void astraeus_command_assign_mesh(EngineHandle engine, uint32_t entity_id, uint32_t mesh_id);

/**
 * Submit an assign material command.
 * @param engine Engine handle
 * @param entity_id Target entity
 * @param material_id Material to assign
 */
ASTRAEUS_API void astraeus_command_assign_material(EngineHandle engine, uint32_t entity_id, uint32_t material_id);

/**
 * Submit a set trail parameters command.
 * @param engine Engine handle
 * @param entity_id Target entity
 * @param max_points Maximum trail points
 */
ASTRAEUS_API void astraeus_command_set_trail(EngineHandle engine, uint32_t entity_id, uint32_t max_points);

/**
 * Submit a set entity color command.
 * @param engine Engine handle
 * @param entity_id Target entity
 * @param r, g, b, a RGBA color components [0.0, 1.0]
 */
ASTRAEUS_API void astraeus_command_set_color(EngineHandle engine, uint32_t entity_id,
                                float r, float g, float b, float a);

/**
 * Submit a set entity visible command.
 * @param engine Engine handle
 * @param entity_id Target entity
 * @param visible Visibility flag
 */
ASTRAEUS_API void astraeus_command_set_visible(EngineHandle engine, uint32_t entity_id, bool visible);

/**
 * Get number of pending commands in queue.
 * @param engine Engine handle
 * @return Number of pending commands
 */
ASTRAEUS_API uint32_t astraeus_command_pending_count(EngineHandle engine);

// =============================================================================
// EVENT BUS API
// =============================================================================

/**
 * Event structure for FFM polling.
 * Events are allocated by engine and must be freed by caller.
 */
typedef struct {
    uint32_t event_type;       // EventType enum value
    uint64_t timestamp_ns;     // Event timestamp
    uint32_t entity_id;        // Entity ID (for entity events)
    float world_x, world_y, world_z;  // World position (for selection events)
    uint32_t data1, data2;     // Generic data fields
    char message[256];         // Message string
} AstraeusEvent;

/**
 * Poll next event from event bus.
 * Returns NULL if no events pending.
 * Caller must free returned event with astraeus_event_free().
 * @param engine Engine handle
 * @return Event pointer or NULL
 */
ASTRAEUS_API AstraeusEvent* astraeus_event_poll(EngineHandle engine);

/**
 * Peek at next event without removing it.
 * Returns NULL if no events pending.
 * DO NOT free the returned pointer.
 * @param engine Engine handle
 * @return Event pointer or NULL
 */
ASTRAEUS_API const AstraeusEvent* astraeus_event_peek(EngineHandle engine);

/**
 * Free an event returned by astraeus_event_poll().
 * @param event Event to free
 */
ASTRAEUS_API void astraeus_event_free(AstraeusEvent* event);

/**
 * Get number of pending events in queue.
 * @param engine Engine handle
 * @return Number of pending events
 */
ASTRAEUS_API uint32_t astraeus_event_pending_count(EngineHandle engine);

/**
 * Clear all pending events.
 * @param engine Engine handle
 */
ASTRAEUS_API void astraeus_event_clear(EngineHandle engine);

// =============================================================================
// PLUGIN API
// =============================================================================

/**
 * Load a plugin from shared library.
 * @param engine Engine handle
 * @param plugin_path Path to plugin shared library (.so, .dll, .dylib)
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_plugin_load(EngineHandle engine, const char* plugin_path);

/**
 * Unload a plugin by name.
 * @param engine Engine handle
 * @param plugin_name Name of plugin to unload
 * @return ASTRAEUS_SUCCESS on success, error code otherwise
 */
ASTRAEUS_API AstraeusResult astraeus_plugin_unload(EngineHandle engine, const char* plugin_name);

/**
 * Get number of loaded plugins.
 * @param engine Engine handle
 * @return Number of loaded plugins
 */
ASTRAEUS_API uint32_t astraeus_plugin_count(EngineHandle engine);

/**
 * Get plugin name by index.
 * @param engine Engine handle
 * @param index Plugin index (0 to count-1)
 * @param out_name_buffer Output buffer for plugin name
 * @param name_buffer_size Size of name buffer in bytes
 * @return true if plugin exists, false otherwise
 */
ASTRAEUS_API bool astraeus_plugin_get_name(EngineHandle engine, uint32_t index,
                              char* out_name_buffer, uint32_t name_buffer_size);

#ifdef __cplusplus
}
#endif

#endif // ASTRAEUS_ENGINE_API_H
