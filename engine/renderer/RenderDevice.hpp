#ifndef ASTRAEUS_RENDER_DEVICE_HPP
#define ASTRAEUS_RENDER_DEVICE_HPP

#include <cstdint>
#include <vector>
#include <iostream>
#include <cstring>
#include "core/util/Math.hpp"
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

// ============================================================================
// Inline implementations
// ============================================================================

inline RenderDevice::RenderDevice(const Config& config)
    : config_(config)
    , width_(config.width)
    , height_(config.height)
    , stats_{}  // Zero-initialize with brace initialization
    , is_initialized_(false)
    , readback_configured_(false)
{
}

inline RenderDevice::~RenderDevice() {
    shutdown();
}

inline bool RenderDevice::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[RenderDevice] Initializing " << width_ << "x" << height_ << std::endl;
    
    // Configure default readback buffers if not already configured
    if (!readback_configured_) {
        ReadbackConfig default_color;
        default_color.max_width = math::max(width_, 2560u);  // Default to 2560x1440 max
        default_color.max_height = math::max(height_, 1440u);
        default_color.format = PIXEL_FORMAT_BGRA8;
        default_color.enable_double_buffer = false;
        
        ReadbackConfig default_id;
        default_id.max_width = default_color.max_width;
        default_id.max_height = default_color.max_height;
        default_id.format = PIXEL_FORMAT_R32UI;
        default_id.enable_double_buffer = false;
        
        configure_readback(&default_color, &default_id);
    }
    
    // TODO: Initialize actual OpenGL context or other backend
    // For now, this is a stub implementation
    
    is_initialized_ = true;
    return true;
}

inline void RenderDevice::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[RenderDevice] Shutting down" << std::endl;
    
    // Clear backing buffers
    color_backing_.data.clear();
    color_backing_.back_buffer.clear();
    id_backing_.data.clear();
    id_backing_.back_buffer.clear();
    
    // TODO: Cleanup GPU resources
    
    is_initialized_ = false;
}

inline void RenderDevice::begin_frame() {
    stats_.draw_calls = 0;
    stats_.triangle_count = 0;
}

inline void RenderDevice::end_frame() {
    // TODO: Present frame
    stats_.render_time_ms = 16.67; // Placeholder ~60fps
    
    // Swap double buffers if enabled
    if (color_backing_.double_buffered) {
        color_backing_.front_buffer_active = !color_backing_.front_buffer_active;
    }
    if (id_backing_.double_buffered) {
        id_backing_.front_buffer_active = !id_backing_.front_buffer_active;
    }
}

inline void RenderDevice::resize(uint32_t width, uint32_t height) {
    std::cout << "[RenderDevice] Resizing viewport to " << width << "x" << height 
              << " (backing buffers remain fixed)" << std::endl;
    
    // Check bounds
    if (width > color_backing_.max_width || height > color_backing_.max_height) {
        std::cerr << "[RenderDevice] WARNING: Requested size (" << width << "x" << height 
                  << ") exceeds max backing size (" << color_backing_.max_width << "x" 
                  << color_backing_.max_height << "). Clamping." << std::endl;
        width = math::min(width, color_backing_.max_width);
        height = math::min(height, color_backing_.max_height);
    }
    
    width_ = width;
    height_ = height;
    
    // Update viewport region in backing buffers (no reallocation!)
    color_backing_.current_width = width;
    color_backing_.current_height = height;
    id_backing_.current_width = width;
    id_backing_.current_height = height;
    
    // TODO: Update GPU framebuffer viewport (not size)
}

inline bool RenderDevice::configure_readback(const ReadbackConfig* color_config, 
                                       const ReadbackConfig* id_config) {
    if (readback_configured_) {
        std::cerr << "[RenderDevice] Readback already configured. Ignoring." << std::endl;
        return false;
    }
    
    // Configure color buffer
    if (color_config) {
        color_backing_.max_width = color_config->max_width;
        color_backing_.max_height = color_config->max_height;
        color_backing_.current_width = std::min(width_, color_backing_.max_width);
        color_backing_.current_height = std::min(height_, color_backing_.max_height);
        color_backing_.format = color_config->format;
        color_backing_.double_buffered = color_config->enable_double_buffer;
        
        // Calculate bytes per pixel based on format
        uint32_t bytes_per_pixel = 4;  // Default for most formats
        if (color_config->format == PIXEL_FORMAT_R32UI) {
            bytes_per_pixel = 4;  // 32-bit unsigned int
        } else {
            bytes_per_pixel = 4;  // RGBA8, BGRA8, ARGB8 all use 4 bytes
        }
        
        size_t buffer_size = color_backing_.max_width * color_backing_.max_height * bytes_per_pixel;
        
        color_backing_.data.resize(buffer_size, 0);
        if (color_backing_.double_buffered) {
            color_backing_.back_buffer.resize(buffer_size, 0);
        }
        color_backing_.front_buffer_active = true;
        
        std::cout << "[RenderDevice] Color buffer configured: " 
                  << color_backing_.max_width << "x" << color_backing_.max_height
                  << " (" << buffer_size << " bytes, double_buffer=" 
                  << color_backing_.double_buffered << ")" << std::endl;
    }
    
    // Configure ID buffer
    if (id_config) {
        id_backing_.max_width = id_config->max_width;
        id_backing_.max_height = id_config->max_height;
        id_backing_.current_width = math::min(width_, id_backing_.max_width);
        id_backing_.current_height = math::min(height_, id_backing_.max_height);
        id_backing_.format = id_config->format;
        id_backing_.double_buffered = id_config->enable_double_buffer;
        
        uint32_t bytes_per_pixel = 4; // R32UI
        size_t buffer_size = id_backing_.max_width * id_backing_.max_height * bytes_per_pixel;
        
        id_backing_.data.resize(buffer_size, 0);
        if (id_backing_.double_buffered) {
            id_backing_.back_buffer.resize(buffer_size, 0);
        }
        id_backing_.front_buffer_active = true;
        
        std::cout << "[RenderDevice] ID buffer configured: " 
                  << id_backing_.max_width << "x" << id_backing_.max_height
                  << " (" << buffer_size << " bytes, double_buffer=" 
                  << id_backing_.double_buffered << ")" << std::endl;
    }
    
    readback_configured_ = true;
    return true;
}

inline void RenderDevice::get_color_buffer_view(PixelBufferView& out_view) const {
    // Return view to STABLE backing buffer (never changes pointer)
    // Always use front buffer if double buffering is disabled
    const auto& buffer = (color_backing_.double_buffered && !color_backing_.front_buffer_active) ? 
                         color_backing_.back_buffer : color_backing_.data;
    
    out_view.data = const_cast<void*>(static_cast<const void*>(buffer.data()));
    out_view.width = color_backing_.current_width;
    out_view.height = color_backing_.current_height;
    out_view.stride = color_backing_.max_width * 4;  // Full backing stride, not viewport
    out_view.format = color_backing_.format;
    out_view.max_backing_width = color_backing_.max_width;
    out_view.max_backing_height = color_backing_.max_height;
    out_view.max_backing_size = static_cast<uint32_t>(buffer.size());
}

inline void RenderDevice::get_id_buffer_view(PixelBufferView& out_view) const {
    // Return view to STABLE backing buffer (never changes pointer)
    // Always use front buffer if double buffering is disabled
    const auto& buffer = (id_backing_.double_buffered && !id_backing_.front_buffer_active) ? 
                         id_backing_.back_buffer : id_backing_.data;
    
    out_view.data = const_cast<void*>(static_cast<const void*>(buffer.data()));
    out_view.width = id_backing_.current_width;
    out_view.height = id_backing_.current_height;
    out_view.stride = id_backing_.max_width * 4;  // R32UI
    out_view.format = id_backing_.format;
    out_view.max_backing_width = id_backing_.max_width;
    out_view.max_backing_height = id_backing_.max_height;
    out_view.max_backing_size = static_cast<uint32_t>(buffer.size());
}

inline void RenderDevice::pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result) const {
    // TODO: Implement actual picking from ID buffer
    (void)screen_x;  // Unused for now
    (void)screen_y;  // Unused for now
    out_result.entity_id = 0;
    out_result.depth = 1.0f;
    out_result.world_x = 0.0f;
    out_result.world_y = 0.0f;
    out_result.world_z = 0.0f;
    out_result.hit = false;
}

} // namespace astraeus

#endif // ASTRAEUS_RENDER_DEVICE_HPP
