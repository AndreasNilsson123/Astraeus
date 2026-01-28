// Minimal implementation file for C API
// Provides non-inline definitions for C linkage

#include "EngineAPI.h"
#include "core/EngineContext.hpp"
#include <cstring>
#include <memory>

// Internal structure definition
struct AstraeusEngine {
    std::unique_ptr<astraeus::EngineContext> context;
    FrameStats current_stats;
    ViewportConfig viewport;
    bool is_initialized;
    
    AstraeusEngine() : is_initialized(false) {
        memset(&current_stats, 0, sizeof(FrameStats));
        memset(&viewport, 0, sizeof(ViewportConfig));
    }
};

// Helper function
static inline bool is_valid_engine(EngineHandle engine) {
    return engine != nullptr && engine->is_initialized && engine->context != nullptr;
}

// =============================================================================
// C API IMPLEMENTATIONS (non-inline for C linkage)
// =============================================================================

extern "C" {

EngineHandle astraeus_create_engine(const EngineConfig* config) {
    if (!config) {
        return nullptr;
    }
    
    try {
        auto engine = new AstraeusEngine();
        
        astraeus::EngineContext::Config ctx_config;
        ctx_config.initial_width = config->initial_width;
        ctx_config.initial_height = config->initial_height;
        ctx_config.enable_validation = config->enable_validation;
        ctx_config.enable_debug_output = config->enable_debug_output;
        if (config->log_file_path) {
            ctx_config.log_file_path = config->log_file_path;
        }
        
        engine->context = std::make_unique<astraeus::EngineContext>(ctx_config);
        engine->is_initialized = engine->context->initialize();
        
        if (!engine->is_initialized) {
            delete engine;
            return nullptr;
        }
        
        engine->viewport.width = config->initial_width;
        engine->viewport.height = config->initial_height;
        engine->viewport.aspect_ratio = static_cast<float>(config->initial_width) / 
                                       static_cast<float>(config->initial_height);
        
        return engine;
    } catch (...) {
        return nullptr;
    }
}

void astraeus_destroy_engine(EngineHandle engine) {
    if (engine) {
        if (engine->context) {
            engine->context->shutdown();
        }
        delete engine;
    }
}

bool astraeus_is_valid(EngineHandle engine) {
    return is_valid_engine(engine);
}

void astraeus_begin_frame(EngineHandle engine, double delta_time) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->begin_frame(delta_time);
}

void astraeus_end_frame(EngineHandle engine) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->end_frame();
    
    // Update frame stats
    engine->current_stats.frame_number++;
    engine->context->get_frame_stats(engine->current_stats);
}

void astraeus_resize_viewport(EngineHandle engine, uint32_t width, uint32_t height) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->viewport.width = width;
    engine->viewport.height = height;
    engine->viewport.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    
    engine->context->resize_viewport(width, height);
}

bool astraeus_configure_readback(EngineHandle engine, 
                                  const ReadbackConfig* color_config,
                                  const ReadbackConfig* id_config) {
    if (!is_valid_engine(engine)) {
        return false;
    }
    
    return engine->context->configure_readback(color_config, id_config);
}

void astraeus_get_frame_stats(EngineHandle engine, FrameStats* out_stats) {
    if (!is_valid_engine(engine) || !out_stats) {
        return;
    }
    
    *out_stats = engine->current_stats;
}

void astraeus_get_color_buffer(EngineHandle engine, PixelBufferView* out_view) {
    if (!out_view) return;

    // Initialize to defaults with RGBA8 format
    *out_view = {nullptr, 0, 0, 0, PIXEL_FORMAT_RGBA8, 0, 0, 0};

    if (!is_valid_engine(engine)) return;

    engine->context->get_color_buffer_view(*out_view);
}

void astraeus_get_id_buffer(EngineHandle engine, PixelBufferView* out_view) {
    if (!out_view) return;
    
    // Initialize to defaults with ID buffer format
    *out_view = {nullptr, 0, 0, 0, PIXEL_FORMAT_R32UI, 0, 0, 0};
    
    if (!is_valid_engine(engine)) return;
    
    engine->context->get_id_buffer_view(*out_view);
}

uint32_t astraeus_create_entity(EngineHandle engine) {
    if (!is_valid_engine(engine)) {
        return 0;
    }
    
    return engine->context->create_entity();
}

void astraeus_destroy_entity(EngineHandle engine, uint32_t entity_id) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->destroy_entity(entity_id);
}

void astraeus_set_entity_transform(EngineHandle engine, uint32_t entity_id,
                                   float pos_x, float pos_y, float pos_z,
                                   float rot_x, float rot_y, float rot_z,
                                   float scale_x, float scale_y, float scale_z) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->set_entity_transform(entity_id, 
                                         pos_x, pos_y, pos_z,
                                         rot_x, rot_y, rot_z,
                                         scale_x, scale_y, scale_z);
}

void astraeus_set_entity_renderable(EngineHandle engine, uint32_t entity_id, bool visible) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->set_entity_renderable(entity_id, visible);
}

void astraeus_set_entity_color(EngineHandle engine, uint32_t entity_id,
                               float r, float g, float b, float a) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->set_entity_color(entity_id, r, g, b, a);
}

void astraeus_set_entity_trail(EngineHandle engine, uint32_t entity_id, uint32_t max_points) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->set_entity_trail(entity_id, max_points);
}

void astraeus_apply_entity_snapshot(EngineHandle engine, uint32_t entity_id,
                                    float pos_x, float pos_y, float pos_z) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->apply_entity_snapshot(entity_id, pos_x, pos_y, pos_z);
}

PickResult astraeus_pick(EngineHandle engine, uint32_t screen_x, uint32_t screen_y) {
    PickResult result = {0, 0.0f, 0.0f, 0.0f, 0.0f, false};
    
    if (!is_valid_engine(engine)) {
        return result;
    }
    
    engine->context->pick(screen_x, screen_y, result);
    return result;
}

bool astraeus_ingest_data(EngineHandle engine, const void* data, uint32_t size, uint32_t format) {
    if (!is_valid_engine(engine) || !data) {
        return false;
    }
    
    return engine->context->ingest_data(data, size, format);
}

void astraeus_set_camera(EngineHandle engine,
                        float eye_x, float eye_y, float eye_z,
                        float target_x, float target_y, float target_z,
                        float up_x, float up_y, float up_z) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->set_camera(eye_x, eye_y, eye_z,
                               target_x, target_y, target_z,
                               up_x, up_y, up_z);
}

void astraeus_set_camera_projection(EngineHandle engine, float fov_degrees, float near_plane, float far_plane) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->set_camera_projection(fov_degrees, near_plane, far_plane);
}

} // extern "C"

