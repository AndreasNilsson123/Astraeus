#include "RenderDevice.hpp"
#include "../api/EngineAPI.h"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace astraeus {

RenderDevice::RenderDevice(const Config& config)
    : config_(config)
    , width_(config.width)
    , height_(config.height)
    , is_initialized_(false)
    , readback_configured_(false)
{
    memset(&stats_, 0, sizeof(Stats));
}

RenderDevice::~RenderDevice() {
    shutdown();
}

bool RenderDevice::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[RenderDevice] Initializing " << width_ << "x" << height_ << std::endl;
    
    // Configure default readback buffers if not already configured
    if (!readback_configured_) {
        ReadbackConfig default_color;
        default_color.max_width = std::max(width_, 2560u);  // Default to 2560x1440 max
        default_color.max_height = std::max(height_, 1440u);
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

void RenderDevice::shutdown() {
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

void RenderDevice::begin_frame() {
    stats_.draw_calls = 0;
    stats_.triangle_count = 0;
}

void RenderDevice::end_frame() {
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

void RenderDevice::resize(uint32_t width, uint32_t height) {
    std::cout << "[RenderDevice] Resizing viewport to " << width << "x" << height 
              << " (backing buffers remain fixed)" << std::endl;
    
    // Check bounds
    if (width > color_backing_.max_width || height > color_backing_.max_height) {
        std::cerr << "[RenderDevice] WARNING: Requested size (" << width << "x" << height 
                  << ") exceeds max backing size (" << color_backing_.max_width << "x" 
                  << color_backing_.max_height << "). Clamping." << std::endl;
        width = std::min(width, color_backing_.max_width);
        height = std::min(height, color_backing_.max_height);
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

bool RenderDevice::configure_readback(const ReadbackConfig* color_config, 
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
        id_backing_.current_width = std::min(width_, id_backing_.max_width);
        id_backing_.current_height = std::min(height_, id_backing_.max_height);
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

void RenderDevice::get_color_buffer_view(PixelBufferView& out_view) const {
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

void RenderDevice::get_id_buffer_view(PixelBufferView& out_view) const {
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

void RenderDevice::pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result) const {
    // TODO: Implement actual picking from ID buffer
    out_result.entity_id = 0;
    out_result.depth = 1.0f;
    out_result.world_x = 0.0f;
    out_result.world_y = 0.0f;
    out_result.world_z = 0.0f;
    out_result.hit = false;
}

} // namespace astraeus
