#ifndef ASTRAEUS_RENDER_DEVICE_HPP
#define ASTRAEUS_RENDER_DEVICE_HPP

#include <cstdint>
#include <vector>
#include "../api/EngineAPI.h"

namespace astraeus {

/**
 * Abstract render device that manages GPU resources and rendering.
 * This can be swapped for different backends (OpenGL, Vulkan, etc.)
 */
class RenderDevice {
public:
    struct Config {
        uint32_t width = 1280;
        uint32_t height = 720;
        bool enable_validation = true;
        bool enable_debug = false;
    };

    struct Stats {
        double render_time_ms = 0.0;
        uint32_t draw_calls = 0;
        uint32_t triangle_count = 0;
    };

    explicit RenderDevice(const Config& config);
    virtual ~RenderDevice();

    virtual bool initialize();
    virtual void shutdown();

    virtual void begin_frame();
    virtual void end_frame();
    virtual void resize(uint32_t width, uint32_t height);
    
    /**
     * Configure readback buffers with fixed backing size.
     * Must be called before first use.
     */
    virtual bool configure_readback(const ReadbackConfig* color_config, 
                                     const ReadbackConfig* id_config);

    virtual void get_color_buffer_view(PixelBufferView& out_view) const;
    virtual void get_id_buffer_view(PixelBufferView& out_view) const;
    virtual void pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result) const;

    const Stats& get_stats() const { return stats_; }
    uint32_t get_width() const { return width_; }
    uint32_t get_height() const { return height_; }

protected:
    Config config_;
    uint32_t width_;
    uint32_t height_;
    Stats stats_;
    bool is_initialized_;
    
    // Fixed-size backing buffers for readback (never resized after allocation)
    struct BackingBuffer {
        std::vector<uint8_t> data;
        std::vector<uint8_t> back_buffer;  // For double-buffering
        uint32_t max_width = 0;
        uint32_t max_height = 0;
        uint32_t current_width = 0;
        uint32_t current_height = 0;
        uint32_t format = 0;
        bool double_buffered = false;
        bool front_buffer_active = true;  // Toggle between front/back
    };
    
    BackingBuffer color_backing_;
    BackingBuffer id_backing_;
    bool readback_configured_ = false;
};

} // namespace astraeus

#endif // ASTRAEUS_RENDER_DEVICE_HPP
