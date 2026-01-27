#include "EngineAPI.h"
#include "../core/EngineContext.hpp"
#include <cstring>
#include <memory>

// =============================================================================
// INTERNAL STRUCTURES
// =============================================================================

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

// =============================================================================
// ENGINE LIFECYCLE
// =============================================================================

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
    return engine != nullptr && engine->is_initialized && engine->context != nullptr;
}

// =============================================================================
// RENDERING
// =============================================================================

void astraeus_begin_frame(EngineHandle engine, double delta_time) {
    if (!astraeus_is_valid(engine)) {
        return;
    }
    
    engine->context->begin_frame(delta_time);
}

void astraeus_end_frame(EngineHandle engine) {
    if (!astraeus_is_valid(engine)) {
        return;
    }
    
    engine->context->end_frame();
    
    // Update frame stats
    engine->current_stats.frame_number++;
    // Stats will be filled by the engine context
    engine->context->get_frame_stats(engine->current_stats);
}

void astraeus_resize_viewport(EngineHandle engine, uint32_t width, uint32_t height) {
    if (!astraeus_is_valid(engine)) {
        return;
    }
    
    engine->viewport.width = width;
    engine->viewport.height = height;
    engine->viewport.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    
    engine->context->resize_viewport(width, height);
}

void astraeus_get_frame_stats(EngineHandle engine, FrameStats* out_stats) {
    if (!astraeus_is_valid(engine) || !out_stats) {
        return;
    }
    
    *out_stats = engine->current_stats;
}

PixelBufferView astraeus_get_color_buffer(EngineHandle engine) {
    PixelBufferView view = {nullptr, 0, 0, 0, 0};
    
    if (!astraeus_is_valid(engine)) {
        return view;
    }
    
    engine->context->get_color_buffer_view(view);
    return view;
}

PixelBufferView astraeus_get_id_buffer(EngineHandle engine) {
    PixelBufferView view = {nullptr, 0, 0, 0, 2}; // Format 2 = R32UI
    
    if (!astraeus_is_valid(engine)) {
        return view;
    }
    
    engine->context->get_id_buffer_view(view);
    return view;
}

// =============================================================================
// SCENE MANAGEMENT
// =============================================================================

uint32_t astraeus_create_entity(EngineHandle engine) {
    if (!astraeus_is_valid(engine)) {
        return 0;
    }
    
    return engine->context->create_entity();
}

void astraeus_destroy_entity(EngineHandle engine, uint32_t entity_id) {
    if (!astraeus_is_valid(engine)) {
        return;
    }
    
    engine->context->destroy_entity(entity_id);
}

void astraeus_set_entity_transform(EngineHandle engine, uint32_t entity_id,
                                   float pos_x, float pos_y, float pos_z,
                                   float rot_x, float rot_y, float rot_z,
                                   float scale_x, float scale_y, float scale_z) {
    if (!astraeus_is_valid(engine)) {
        return;
    }
    
    engine->context->set_entity_transform(entity_id, 
                                         pos_x, pos_y, pos_z,
                                         rot_x, rot_y, rot_z,
                                         scale_x, scale_y, scale_z);
}

// =============================================================================
// PICKING
// =============================================================================

PickResult astraeus_pick(EngineHandle engine, uint32_t screen_x, uint32_t screen_y) {
    PickResult result = {0, 0.0f, 0.0f, 0.0f, 0.0f, false};
    
    if (!astraeus_is_valid(engine)) {
        return result;
    }
    
    engine->context->pick(screen_x, screen_y, result);
    return result;
}

// =============================================================================
// DATA INGESTION
// =============================================================================

bool astraeus_ingest_data(EngineHandle engine, const void* data, uint32_t size, uint32_t format) {
    if (!astraeus_is_valid(engine) || !data) {
        return false;
    }
    
    return engine->context->ingest_data(data, size, format);
}

// =============================================================================
// CAMERA CONTROL
// =============================================================================

void astraeus_set_camera(EngineHandle engine,
                        float eye_x, float eye_y, float eye_z,
                        float target_x, float target_y, float target_z,
                        float up_x, float up_y, float up_z) {
    if (!astraeus_is_valid(engine)) {
        return;
    }
    
    engine->context->set_camera(eye_x, eye_y, eye_z,
                               target_x, target_y, target_z,
                               up_x, up_y, up_z);
}

void astraeus_set_camera_projection(EngineHandle engine, float fov_degrees, float near_plane, float far_plane) {
    if (!astraeus_is_valid(engine)) {
        return;
    }
    
    engine->context->set_camera_projection(fov_degrees, near_plane, far_plane);
}
