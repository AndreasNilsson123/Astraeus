#ifndef ASTRAEUS_ENGINE_CONTEXT_HPP
#define ASTRAEUS_ENGINE_CONTEXT_HPP

#include <cstdint>
#include <string>
#include <memory>

// Include C API types
#include "../api/EngineAPI.h"

// Forward declarations
namespace astraeus {
    class RenderDevice;
    class RenderGraph;
    class World;
    class IngestManager;
    class AssetManager;
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

    // Accessors for subsystems (internal use)
    RenderDevice* get_render_device() const { return render_device_.get(); }
    RenderGraph* get_render_graph() const { return render_graph_.get(); }
    World* get_world() const { return world_.get(); }

private:
    Config config_;
    bool is_initialized_;
    
    // Major subsystems
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

} // namespace astraeus

// Include inline implementations if requested
// This should only be included in .cpp files that actually instantiate EngineContext
#ifdef ASTRAEUS_ENGINE_CONTEXT_INLINE_IMPL
#include "EngineContext_impl.hpp"
#endif

#endif // ASTRAEUS_ENGINE_CONTEXT_HPP
