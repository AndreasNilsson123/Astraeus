#ifndef ASTRAEUS_RENDER_DEVICE_HPP
#define ASTRAEUS_RENDER_DEVICE_HPP

#include <cstdint>
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
};

} // namespace astraeus

#endif // ASTRAEUS_RENDER_DEVICE_HPP
