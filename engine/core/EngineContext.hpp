#ifndef ASTRAEUS_ENGINE_CONTEXT_HPP
#define ASTRAEUS_ENGINE_CONTEXT_HPP

#include <cstdint>
#include <string>
#include <memory>
#include <iostream>

// Include C API types
#include "api/EngineAPI.h"

// Include implementation headers for inline methods
#include "core/Telemetry.hpp"
#include "renderer/RenderDevice.hpp"
#include "renderer/opengl/GLRenderDevice.hpp"
#include "renderer/RenderGraph.hpp"
#include "renderer/passes/ClearPass.hpp"
#include "renderer/passes/GridPass.hpp"
#include "renderer/passes/AxesPass.hpp"
#include "scene/World.hpp"
#include "ingest/IngestManager.hpp"
#include "assets/AssetManager.hpp"

// Forward declarations
namespace astraeus {
    class RenderDevice;
    class RenderGraph;
    class World;
    class IngestManager;
    class AssetManager;
    class Telemetry;
}

namespace astraeus {

/**
 * Core engine context that owns all major subsystems.
 * This is the central point of coordination for the entire engine.
 */
class EngineContext {
public:
    struct Config {
        uint32_t initial_width = 1280;
        uint32_t initial_height = 720;
        bool enable_validation = true;
        bool enable_debug_output = false;
        std::string log_file_path;
    };

    explicit EngineContext(const Config& config);
    ~EngineContext();

    // Non-copyable
    EngineContext(const EngineContext&) = delete;
    EngineContext& operator=(const EngineContext&) = delete;

    /**
     * Initialize all engine subsystems.
     * @return true on success, false on failure
     */
    bool initialize();

    /**
     * Shutdown and cleanup all subsystems.
     */
    void shutdown();

    /**
     * Begin a new frame.
     * @param delta_time Time since last frame in seconds
     */
    void begin_frame(double delta_time);

    /**
     * End the current frame and submit rendering.
     */
    void end_frame();

    /**
     * Resize the viewport.
     * @param width New width
     * @param height New height
     */
    void resize_viewport(uint32_t width, uint32_t height);
    
    /**
     * Configure readback buffers with fixed backing size.
     * Must be called before first frame.
     */
    bool configure_readback(const ReadbackConfig* color_config, 
                            const ReadbackConfig* id_config);

    /**
     * Get current frame statistics.
     * @param out_stats Output frame statistics
     */
    void get_frame_stats(FrameStats& out_stats) const;

    /**
     * Get a view of the color buffer for readback.
     * @param out_view Output pixel buffer view
     */
    void get_color_buffer_view(PixelBufferView& out_view) const;

    /**
     * Get a view of the ID buffer for picking.
     * @param out_view Output pixel buffer view
     */
    void get_id_buffer_view(PixelBufferView& out_view) const;

    /**
     * Create a new entity in the scene.
     * @return Entity ID (handle-based)
     */
    uint32_t create_entity();

    /**
     * Destroy an entity.
     * @param entity_id Entity ID
     */
    void destroy_entity(uint32_t entity_id);

    /**
     * Set entity transform.
     */
    void set_entity_transform(uint32_t entity_id,
                             float pos_x, float pos_y, float pos_z,
                             float rot_x, float rot_y, float rot_z,
                             float scale_x, float scale_y, float scale_z);

    /**
     * Perform picking at screen coordinates.
     */
    void pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result);

    /**
     * Ingest external simulation data.
     */
    bool ingest_data(const void* data, uint32_t size, uint32_t format);

    /**
     * Set camera position and target.
     */
    void set_camera(float eye_x, float eye_y, float eye_z,
                   float target_x, float target_y, float target_z,
                   float up_x, float up_y, float up_z);

    /**
     * Set camera projection parameters.
     */
    void set_camera_projection(float fov_degrees, float near_plane, float far_plane);

    /**
     * Set entity renderable (visibility) state.
     */
    void set_entity_renderable(uint32_t entity_id, bool visible);

    /**
     * Set entity color.
     */
    void set_entity_color(uint32_t entity_id, float r, float g, float b, float a);

    /**
     * Set entity trail (enable trail rendering).
     */
    void set_entity_trail(uint32_t entity_id, uint32_t max_points);

    /**
     * Apply entity snapshot at time t (WorldSync entry point).
     */
    void apply_entity_snapshot(uint32_t entity_id, float pos_x, float pos_y, float pos_z);

    /**
     * Enable or disable telemetry collection.
     */
    void set_telemetry_enabled(bool enabled);

    /**
     * Check if telemetry is enabled.
     */
    bool is_telemetry_enabled() const;

    /**
     * Get telemetry statistics for the current frame.
     */
    const Telemetry::FrameStats& get_telemetry_stats() const;

    /**
     * Get telemetry history.
     * @param out_buffer Output buffer for frame stats
     * @param max_frames Maximum number of frames to retrieve
     * @return Number of frames actually written
     */
    uint32_t get_telemetry_history(Telemetry::FrameStats* out_buffer, uint32_t max_frames) const;

    /**
     * Get the number of render passes in the current frame.
     */
    uint32_t get_telemetry_pass_count() const;

    /**
     * Get timing information for a specific pass.
     */
    const Telemetry::PassTiming* get_telemetry_pass_timing(uint32_t pass_index) const;

    // Accessors for subsystems (internal use)
    RenderDevice* get_render_device() const { return render_device_.get(); }
    RenderGraph* get_render_graph() const { return render_graph_.get(); }
    World* get_world() const { return world_.get(); }
    Telemetry* get_telemetry() const { return telemetry_.get(); }
    AssetManager* get_asset_manager() const { return asset_manager_.get(); }

private:
    Config config_;
    bool is_initialized_;
    
    // Major subsystems
    std::unique_ptr<Telemetry> telemetry_;
    std::unique_ptr<RenderDevice> render_device_;
    std::unique_ptr<RenderGraph> render_graph_;
    std::unique_ptr<World> world_;
    std::unique_ptr<IngestManager> ingest_manager_;
    std::unique_ptr<AssetManager> asset_manager_;
    
    // Frame timing
    double current_delta_time_;
    uint64_t frame_count_;
    double total_time_;
};

// =============================================================================
// INLINE IMPLEMENTATIONS
// =============================================================================

inline EngineContext::EngineContext(const Config& config)
    : config_(config)
    , is_initialized_(false)
    , current_delta_time_(0.0)
    , frame_count_(0)
    , total_time_(0.0)
{
}

inline EngineContext::~EngineContext() {
    shutdown();
}

inline bool EngineContext::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[Astraeus] Initializing engine..." << std::endl;

    try {
        // Initialize telemetry system (enabled by default in constructor)
        telemetry_ = std::make_unique<Telemetry>();
        
        // Initialize render device (use GL backend)
        RenderDevice::Config render_config;
        render_config.width = config_.initial_width;
        render_config.height = config_.initial_height;
        render_config.enable_validation = config_.enable_validation;
        render_config.enable_debug = config_.enable_debug_output;
        
        render_device_ = std::make_unique<GLRenderDevice>(render_config);
        if (!render_device_->initialize()) {
            std::cerr << "[Astraeus] Failed to initialize render device" << std::endl;
            return false;
        }

        // Initialize world/scene
        world_ = std::make_unique<World>();
        world_->initialize();

        // Initialize render graph with passes (pass telemetry for per-pass timing)
        render_graph_ = std::make_unique<RenderGraph>(render_device_.get(), world_.get(), telemetry_.get());
        render_graph_->initialize();
        
        // Add render passes: Clear, Grid, Axes
        render_graph_->add_pass(std::make_unique<ClearPass>());
        render_graph_->add_pass(std::make_unique<GridPass>());
        render_graph_->add_pass(std::make_unique<AxesPass>());

        // Initialize ingest manager
        ingest_manager_ = std::make_unique<IngestManager>(world_.get());
        ingest_manager_->initialize();

        // Initialize asset manager
        asset_manager_ = std::make_unique<AssetManager>(render_device_.get());
        asset_manager_->initialize();

        is_initialized_ = true;
        std::cout << "[Astraeus] Engine initialized successfully" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "[Astraeus] Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

inline void EngineContext::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[Astraeus] Shutting down engine..." << std::endl;

    // Shutdown in reverse order
    asset_manager_.reset();
    ingest_manager_.reset();
    render_graph_.reset();
    world_.reset();
    render_device_.reset();
    telemetry_.reset();

    is_initialized_ = false;
    std::cout << "[Astraeus] Engine shutdown complete" << std::endl;
}

inline void EngineContext::begin_frame(double delta_time) {
    current_delta_time_ = delta_time;
    total_time_ += delta_time;
    
    // Begin telemetry frame timing
    if (telemetry_) {
        telemetry_->begin_frame(frame_count_);
    }
    
    // Update camera matrices in render device for picking
    if (render_device_ && world_) {
        const Camera& camera = world_->get_camera();
        const float* vp_matrix = camera.get_view_projection_matrix();
        
        // Update render device with current camera matrices
        // Uses virtual method (no dynamic_cast needed)
        if (vp_matrix) {
            render_device_->set_view_projection_matrix(vp_matrix);
        }
    }
    
    if (render_device_) {
        render_device_->begin_frame();
    }
}

inline void EngineContext::end_frame() {
    // Process pending GPU uploads
    if (asset_manager_) {
        asset_manager_->process_uploads();
    }
    
    if (render_graph_) {
        render_graph_->execute();
    }
    
    if (render_device_) {
        render_device_->end_frame();
    }
    
    // End telemetry frame timing
    if (telemetry_) {
        auto device_stats = render_device_ ? render_device_->get_stats() : RenderDevice::Stats{};
        telemetry_->end_frame(device_stats.draw_calls, device_stats.triangle_count);
    }
    
    frame_count_++;
}

inline void EngineContext::resize_viewport(uint32_t width, uint32_t height) {
    if (render_device_) {
        render_device_->resize(width, height);
    }
    if (render_graph_) {
        render_graph_->on_resize(width, height);
    }
}

inline bool EngineContext::configure_readback(const ReadbackConfig* color_config, 
                                        const ReadbackConfig* id_config) {
    if (render_device_) {
        return render_device_->configure_readback(color_config, id_config);
    }
    return false;
}

inline void EngineContext::get_frame_stats(FrameStats& out_stats) const {
    out_stats.frame_number = frame_count_;
    out_stats.delta_time_ms = current_delta_time_ * 1000.0;
    
    if (render_device_) {
        auto device_stats = render_device_->get_stats();
        out_stats.render_time_ms = device_stats.render_time_ms;
        out_stats.draw_calls = device_stats.draw_calls;
        out_stats.triangle_count = device_stats.triangle_count;
    }
    
    if (world_) {
        out_stats.entity_count = world_->get_entity_count();
    }
}

inline void EngineContext::get_color_buffer_view(PixelBufferView& out_view) const {
    if (render_device_) {
        render_device_->get_color_buffer_view(out_view);
    }
}

inline void EngineContext::get_id_buffer_view(PixelBufferView& out_view) const {
    if (render_device_) {
        render_device_->get_id_buffer_view(out_view);
    }
}

inline uint32_t EngineContext::create_entity() {
    if (world_) {
        return world_->create_entity();
    }
    return 0;
}

inline void EngineContext::destroy_entity(uint32_t entity_id) {
    if (world_) {
        world_->destroy_entity(entity_id);
    }
}

inline void EngineContext::set_entity_transform(uint32_t entity_id,
                                        float pos_x, float pos_y, float pos_z,
                                        float rot_x, float rot_y, float rot_z,
                                        float scale_x, float scale_y, float scale_z) {
    if (world_) {
        world_->set_entity_transform(entity_id,
                                    pos_x, pos_y, pos_z,
                                    rot_x, rot_y, rot_z,
                                    scale_x, scale_y, scale_z);
    }
}

inline void EngineContext::pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result) {
    if (render_device_) {
        render_device_->pick(screen_x, screen_y, out_result);
    }
}

inline bool EngineContext::ingest_data(const void* data, uint32_t size, uint32_t format) {
    if (ingest_manager_) {
        return ingest_manager_->ingest(data, size, format);
    }
    return false;
}

inline void EngineContext::set_camera(float eye_x, float eye_y, float eye_z,
                              float target_x, float target_y, float target_z,
                              float up_x, float up_y, float up_z) {
    if (world_) {
        world_->set_camera(eye_x, eye_y, eye_z,
                          target_x, target_y, target_z,
                          up_x, up_y, up_z);
    }
}

inline void EngineContext::set_camera_projection(float fov_degrees, float near_plane, float far_plane) {
    if (world_) {
        world_->set_camera_projection(fov_degrees, near_plane, far_plane);
    }
}

inline void EngineContext::set_entity_renderable(uint32_t entity_id, bool visible) {
    if (world_) {
        world_->set_entity_renderable(entity_id, visible);
    }
}

inline void EngineContext::set_entity_color(uint32_t entity_id, float r, float g, float b, float a) {
    if (world_) {
        world_->set_entity_color(entity_id, r, g, b, a);
    }
}

inline void EngineContext::set_entity_trail(uint32_t entity_id, uint32_t max_points) {
    if (world_) {
        world_->set_entity_trail(entity_id, max_points);
    }
}

inline void EngineContext::apply_entity_snapshot(uint32_t entity_id, float pos_x, float pos_y, float pos_z) {
    if (world_) {
        world_->apply_entity_snapshot(entity_id, pos_x, pos_y, pos_z);
    }
}

inline void EngineContext::set_telemetry_enabled(bool enabled) {
    if (telemetry_) {
        telemetry_->set_enabled(enabled);
    }
}

inline bool EngineContext::is_telemetry_enabled() const {
    return telemetry_ ? telemetry_->is_enabled() : false;
}

inline const Telemetry::FrameStats& EngineContext::get_telemetry_stats() const {
    static Telemetry::FrameStats empty_stats{};
    return telemetry_ ? telemetry_->get_current_stats() : empty_stats;
}

inline uint32_t EngineContext::get_telemetry_history(Telemetry::FrameStats* out_buffer, uint32_t max_frames) const {
    return telemetry_ ? telemetry_->get_history(out_buffer, max_frames) : 0;
}

inline uint32_t EngineContext::get_telemetry_pass_count() const {
    return telemetry_ ? telemetry_->get_pass_count() : 0;
}

inline const Telemetry::PassTiming* EngineContext::get_telemetry_pass_timing(uint32_t pass_index) const {
    return telemetry_ ? telemetry_->get_pass_timing(pass_index) : nullptr;
}

} // namespace astraeus

#endif // ASTRAEUS_ENGINE_CONTEXT_HPP
