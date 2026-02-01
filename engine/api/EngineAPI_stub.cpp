// Minimal implementation file for C API
// Provides non-inline definitions for C linkage

#include "api/EngineAPI_Internal.hpp"
#include "EngineAPI.h"
#include "core/EngineContext.hpp"
#include "core/util/SafeC.hpp"
#include <cstring>
#include <memory>
#include <type_traits>

// Compile-time check: Ensure C API struct is compatible with internal struct
static_assert(sizeof(TelemetryFrameStats) == sizeof(astraeus::Telemetry::FrameStats),
              "TelemetryFrameStats must match Telemetry::FrameStats size");
static_assert(offsetof(TelemetryFrameStats, frame_number) == offsetof(astraeus::Telemetry::FrameStats, frame_number),
              "frame_number offset mismatch");
static_assert(offsetof(TelemetryFrameStats, pass_count) == offsetof(astraeus::Telemetry::FrameStats, pass_count),
              "pass_count offset mismatch");

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
        engine->viewport.aspect_ratio = static_cast<float>(config->initial_width) / static_cast<float>(config->initial_height);
        
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

// NOTE: astraeus_begin_frame and astraeus_end_frame are now implemented in EngineAPI_RenderSession.cpp
// They have been updated to return AstraeusResult instead of void

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

void astraeus_pick(EngineHandle engine, uint32_t screen_x, uint32_t screen_y, PickResult* pick_result) {
    if (!pick_result)
        return;
    
    // Initialize all fields (including padding) to zero
    *pick_result = {};
    
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->pick(screen_x, screen_y, *pick_result);
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

// =============================================================================
// TELEMETRY IMPLEMENTATIONS
// =============================================================================

void astraeus_enable_telemetry(EngineHandle engine, bool enabled) {
    if (!is_valid_engine(engine)) {
        return;
    }
    
    engine->context->set_telemetry_enabled(enabled);
}

bool astraeus_is_telemetry_enabled(EngineHandle engine) {
    if (!is_valid_engine(engine)) {
        return false;
    }
    
    return engine->context->is_telemetry_enabled();
}

void astraeus_get_telemetry_frame_stats(EngineHandle engine, TelemetryFrameStats* out_stats) {
    if (!is_valid_engine(engine) || !out_stats) {
        return;
    }
    
    const auto& stats = engine->context->get_telemetry_stats();
    out_stats->frame_number = stats.frame_number;
    out_stats->cpu_time_ms = stats.cpu_time_ms;
    out_stats->gpu_time_ms = stats.gpu_time_ms;
    out_stats->total_time_ms = stats.total_time_ms;
    out_stats->draw_calls = stats.draw_calls;
    out_stats->triangle_count = stats.triangle_count;
    out_stats->pass_count = stats.pass_count;
    // _padding is explicitly zero-initialized in struct
}

uint32_t astraeus_get_telemetry_history(EngineHandle engine, TelemetryFrameStats* out_buffer, uint32_t max_frames) {
    if (!is_valid_engine(engine) || !out_buffer || max_frames == 0) {
        return 0;
    }
    
    // Allocate temporary buffer for history retrieval
    auto temp_buffer = std::make_unique<astraeus::Telemetry::FrameStats[]>(max_frames);
    uint32_t count = engine->context->get_telemetry_history(temp_buffer.get(), max_frames);
    
    // Copy field-by-field to ensure ABI safety
    for (uint32_t i = 0; i < count; ++i) {
        const auto& src = temp_buffer[i];
        auto& dst = out_buffer[i];
        dst.frame_number = src.frame_number;
        dst.cpu_time_ms = src.cpu_time_ms;
        dst.gpu_time_ms = src.gpu_time_ms;
        dst.total_time_ms = src.total_time_ms;
        dst.draw_calls = src.draw_calls;
        dst.triangle_count = src.triangle_count;
        dst.pass_count = src.pass_count;
    }
    
    return count;
}

uint32_t astraeus_get_pass_count(EngineHandle engine) {
    if (!is_valid_engine(engine)) {
        return 0;
    }
    
    return engine->context->get_telemetry_pass_count();
}

bool astraeus_get_pass_timing(EngineHandle engine, uint32_t pass_index, 
                               char* out_name_buffer, uint32_t name_buffer_size, 
                               double* out_time_ms) {
    if (!is_valid_engine(engine) || !out_name_buffer || !out_time_ms || name_buffer_size == 0) {
        return false;
    }
    
    const auto* pass_timing = engine->context->get_telemetry_pass_timing(pass_index);
    if (!pass_timing) {
        return false;
    }
    
    // Copy pass name safely using our safe wrapper
    astraeus::util::str_copy(out_name_buffer, name_buffer_size, pass_timing->name);
    
    // Copy timing
    *out_time_ms = pass_timing->duration_ms;
    
    return true;
}

} // extern "C"

