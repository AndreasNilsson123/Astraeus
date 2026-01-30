// Render Session API implementation
// Provides viewport, camera, and material management for Java FFM integration

#include "EngineAPI.h"
#include "core/EngineContext.hpp"
#include "scene/Camera.hpp"
#include "scene/CameraSystem.hpp"
#include "renderer/Material.hpp"
#include "renderer/MaterialLibrary.hpp"
#include <cstring>
#include <string>
#include <thread>
#include <atomic>

// Thread-local error storage for detailed error messages
thread_local std::string last_error_message;

// Internal structure definitions
struct AstraeusViewport {
    astraeus::EngineContext* engine_ctx;  // Non-owning pointer (owned by AstraeusEngine)
    uint32_t width;
    uint32_t height;
    
    AstraeusViewport(astraeus::EngineContext* ctx, uint32_t w, uint32_t h) 
        : engine_ctx(ctx), width(w), height(h) {}
};

struct AstraeusCamera {
    astraeus::EngineContext* engine_ctx;  // Non-owning pointer
    // For MVP, we use the active camera from the world
    // In future, this could be a specific camera ID
    
    AstraeusCamera(astraeus::EngineContext* ctx) : engine_ctx(ctx) {}
};

struct AstraeusMaterial {
    astraeus::EngineContext* engine_ctx;  // Non-owning pointer
    uint32_t material_id;  // Material ID in MaterialLibrary
    
    AstraeusMaterial(astraeus::EngineContext* ctx, uint32_t id) 
        : engine_ctx(ctx), material_id(id) {}
};

// Helper functions
namespace {
    
void set_error(const std::string& msg) {
    last_error_message = msg;
}

bool is_valid_viewport(ViewportHandle viewport) {
    return viewport != nullptr && viewport->engine_ctx != nullptr;
}

bool is_valid_camera(CameraHandle camera) {
    return camera != nullptr && camera->engine_ctx != nullptr;
}

bool is_valid_material(MaterialHandle material) {
    return material != nullptr && material->engine_ctx != nullptr;
}

} // anonymous namespace

// Forward declaration of engine handle struct (defined in EngineAPI_stub.cpp)
struct AstraeusEngine {
    std::unique_ptr<astraeus::EngineContext> context;
    FrameStats current_stats;
    ViewportConfig viewport;
    bool is_initialized;
};

// Helper to check engine validity (matches EngineAPI_stub.cpp)
static inline bool is_valid_engine(EngineHandle engine) {
    return engine != nullptr && engine->is_initialized && engine->context != nullptr;
}

// =============================================================================
// C API IMPLEMENTATIONS
// =============================================================================

extern "C" {

// -----------------------------------------------------------------------------
// API Versioning
// -----------------------------------------------------------------------------

uint32_t astraeus_api_version(void) {
    // Version format: (MAJOR << 16) | (MINOR << 8) | PATCH
    return (ASTRAEUS_VERSION_MAJOR << 16) | 
           (ASTRAEUS_VERSION_MINOR << 8) | 
           ASTRAEUS_VERSION_PATCH;
}

const char* astraeus_last_error(EngineHandle engine) {
    (void)engine;  // Engine param reserved for per-engine error storage in future
    if (last_error_message.empty()) {
        return nullptr;
    }
    return last_error_message.c_str();
}

// -----------------------------------------------------------------------------
// Frame Control (updated to return result codes)
// -----------------------------------------------------------------------------

AstraeusResult astraeus_begin_frame(EngineHandle engine, double delta_time) {
    if (!is_valid_engine(engine)) {
        set_error("Invalid engine handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    try {
        engine->context->begin_frame(delta_time);
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to begin frame: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

AstraeusResult astraeus_end_frame(EngineHandle engine) {
    if (!is_valid_engine(engine)) {
        set_error("Invalid engine handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    try {
        engine->context->end_frame();
        
        // Update frame stats
        engine->current_stats.frame_number++;
        engine->context->get_frame_stats(engine->current_stats);
        
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to end frame: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

// -----------------------------------------------------------------------------
// Viewport API
// -----------------------------------------------------------------------------

AstraeusResult astraeus_viewport_create(EngineHandle engine, 
                                        const ViewportConfig* config, 
                                        ViewportHandle* out_viewport) {
    if (!is_valid_engine(engine)) {
        set_error("Invalid engine handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    if (!config || !out_viewport) {
        set_error("NULL config or out_viewport parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    try {
        // For MVP: viewport is just a wrapper around the engine context
        auto viewport = new AstraeusViewport(engine->context.get(), 
                                             config->width, 
                                             config->height);
        *out_viewport = viewport;
        return ASTRAEUS_SUCCESS;
    } catch (const std::bad_alloc&) {
        set_error("Out of memory creating viewport");
        return ASTRAEUS_ERROR_OUT_OF_MEMORY;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to create viewport: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

AstraeusResult astraeus_viewport_destroy(ViewportHandle viewport) {
    if (!viewport) {
        return ASTRAEUS_SUCCESS;  // NULL is OK for destroy
    }
    
    delete viewport;
    return ASTRAEUS_SUCCESS;
}

AstraeusResult astraeus_viewport_resize(ViewportHandle viewport, 
                                        uint32_t width, uint32_t height) {
    if (!is_valid_viewport(viewport)) {
        set_error("Invalid viewport handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    try {
        viewport->width = width;
        viewport->height = height;
        viewport->engine_ctx->resize_viewport(width, height);
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to resize viewport: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

AstraeusResult astraeus_viewport_get_color(ViewportHandle viewport, 
                                           PixelBufferView* out_view) {
    if (!out_view) {
        set_error("NULL out_view parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    // Initialize to defaults
    *out_view = {nullptr, 0, 0, 0, PIXEL_FORMAT_RGBA8, 0, 0, 0};
    
    if (!is_valid_viewport(viewport)) {
        set_error("Invalid viewport handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    try {
        viewport->engine_ctx->get_color_buffer_view(*out_view);
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to get color buffer: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

AstraeusResult astraeus_viewport_get_idbuffer(ViewportHandle viewport, 
                                              PixelBufferView* out_view) {
    if (!out_view) {
        set_error("NULL out_view parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    // Initialize to defaults
    *out_view = {nullptr, 0, 0, 0, PIXEL_FORMAT_R32UI, 0, 0, 0};
    
    if (!is_valid_viewport(viewport)) {
        set_error("Invalid viewport handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    try {
        viewport->engine_ctx->get_id_buffer_view(*out_view);
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to get ID buffer: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

// -----------------------------------------------------------------------------
// Camera API
// -----------------------------------------------------------------------------

AstraeusResult astraeus_camera_get_active(ViewportHandle viewport, 
                                          CameraHandle* out_camera) {
    if (!is_valid_viewport(viewport)) {
        set_error("Invalid viewport handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    if (!out_camera) {
        set_error("NULL out_camera parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    try {
        // For MVP: return a camera handle wrapping the engine context
        // The actual camera is retrieved from the World's CameraSystem
        // Note: Camera handles are lightweight and do NOT own the camera
        auto camera = new AstraeusCamera(viewport->engine_ctx);
        *out_camera = camera;
        return ASTRAEUS_SUCCESS;
    } catch (const std::bad_alloc&) {
        set_error("Out of memory creating camera handle");
        return ASTRAEUS_ERROR_OUT_OF_MEMORY;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to get active camera: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

/**
 * Destroy a camera handle (does NOT destroy the actual camera).
 */
AstraeusResult astraeus_camera_destroy(CameraHandle camera) {
    if (!camera) {
        return ASTRAEUS_SUCCESS;  // NULL is OK for destroy
    }
    
    delete camera;
    return ASTRAEUS_SUCCESS;
}

AstraeusResult astraeus_camera_get_desc(CameraHandle camera, CameraDesc* out_desc) {
    if (!is_valid_camera(camera)) {
        set_error("Invalid camera handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    if (!out_desc) {
        set_error("NULL out_desc parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    try {
        // Get the camera from World
        auto* world = camera->engine_ctx->get_world();
        if (!world) {
            set_error("No world available");
            return ASTRAEUS_ERROR_NOT_INITIALIZED;
        }
        
        auto& active_camera = world->get_camera();
        
        // Fill out the descriptor
        float pos_x, pos_y, pos_z;
        float target_x, target_y, target_z;
        
        active_camera.get_position(pos_x, pos_y, pos_z);
        active_camera.get_target(target_x, target_y, target_z);
        
        out_desc->pos_x = pos_x;
        out_desc->pos_y = pos_y;
        out_desc->pos_z = pos_z;
        
        out_desc->target_x = target_x;
        out_desc->target_y = target_y;
        out_desc->target_z = target_z;
        
        // Up vector - default to world up (0, 1, 0) since Camera doesn't expose it
        out_desc->up_x = 0.0f;
        out_desc->up_y = 1.0f;
        out_desc->up_z = 0.0f;
        
        out_desc->fov_degrees = active_camera.get_fov();
        out_desc->near_plane = active_camera.get_near_plane();
        out_desc->far_plane = active_camera.get_far_plane();
        
        // Map camera mode (assuming Camera has a mode getter)
        // For now, default to ORBIT mode
        out_desc->mode = CAMERA_MODE_ORBIT;
        
        // Clear padding
        memset(out_desc->_padding, 0, sizeof(out_desc->_padding));
        
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to get camera descriptor: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

AstraeusResult astraeus_camera_set_desc(CameraHandle camera, const CameraDesc* desc) {
    if (!is_valid_camera(camera)) {
        set_error("Invalid camera handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    if (!desc) {
        set_error("NULL desc parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    try {
        // Get the camera from World
        auto* world = camera->engine_ctx->get_world();
        if (!world) {
            set_error("No world available");
            return ASTRAEUS_ERROR_NOT_INITIALIZED;
        }
        
        auto& active_camera = world->get_camera();
        
        // Update camera parameters
        active_camera.set_view(
            desc->pos_x, desc->pos_y, desc->pos_z,
            desc->target_x, desc->target_y, desc->target_z,
            desc->up_x, desc->up_y, desc->up_z
        );
        
        // Get viewport aspect ratio - for MVP we'll use a reasonable default
        // In future, this could come from CameraDesc or viewport dimensions
        float aspect_ratio = 16.0f / 9.0f;  // TODO: Get from viewport or CameraDesc
        
        active_camera.set_projection(desc->fov_degrees, aspect_ratio, 
                                     desc->near_plane, desc->far_plane);
        
        // Note: Camera mode setting would go here if Camera supports it
        // For MVP, we ignore the mode field
        
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to set camera descriptor: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

// -----------------------------------------------------------------------------
// Materials API
// -----------------------------------------------------------------------------

AstraeusResult astraeus_material_create(EngineHandle engine, 
                                        const MaterialDesc* desc, 
                                        MaterialHandle* out_material) {
    if (!is_valid_engine(engine)) {
        set_error("Invalid engine handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    if (!desc || !out_material) {
        set_error("NULL desc or out_material parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    try {
        // For MVP: Create a simple material in the material library
        // In a full implementation, this would interface with MaterialLibrary
        
        // Thread-safe material ID generation
        static std::atomic<uint32_t> next_material_id{1};
        uint32_t material_id = next_material_id.fetch_add(1);
        
        auto material = new AstraeusMaterial(engine->context.get(), material_id);
        *out_material = material;
        
        // TODO: Store material parameters in MaterialLibrary
        // For now, just log the creation
        (void)desc;  // Silence unused parameter warning
        
        return ASTRAEUS_SUCCESS;
    } catch (const std::bad_alloc&) {
        set_error("Out of memory creating material");
        return ASTRAEUS_ERROR_OUT_OF_MEMORY;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to create material: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

AstraeusResult astraeus_material_update(MaterialHandle material, const MaterialDesc* desc) {
    if (!is_valid_material(material)) {
        set_error("Invalid material handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    if (!desc) {
        set_error("NULL desc parameter");
        return ASTRAEUS_ERROR_INVALID_PARAMETER;
    }
    
    try {
        // TODO: Update material in MaterialLibrary
        // For MVP, this is a no-op
        (void)desc;  // Silence unused parameter warning
        
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to update material: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

AstraeusResult astraeus_material_destroy(MaterialHandle material) {
    if (!material) {
        return ASTRAEUS_SUCCESS;  // NULL is OK for destroy
    }
    
    // TODO: Remove material from MaterialLibrary
    
    delete material;
    return ASTRAEUS_SUCCESS;
}

AstraeusResult astraeus_entity_set_material(EngineHandle engine, 
                                            uint32_t entity_id, 
                                            MaterialHandle material) {
    if (!is_valid_engine(engine)) {
        set_error("Invalid engine handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    if (!is_valid_material(material)) {
        set_error("Invalid material handle");
        return ASTRAEUS_ERROR_INVALID_HANDLE;
    }
    
    try {
        // TODO: Assign material to entity in World/EntityRegistry
        // For MVP, this is a no-op
        (void)entity_id;  // Silence unused parameter warning
        
        return ASTRAEUS_SUCCESS;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to set entity material: ") + e.what());
        return ASTRAEUS_ERROR_UNKNOWN;
    }
}

} // extern "C"
