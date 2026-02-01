#ifndef ASTRAEUS_GL_RENDER_DEVICE_HPP
#define ASTRAEUS_GL_RENDER_DEVICE_HPP

#include "renderer/RenderDevice.hpp"
#include "renderer/backend/GraphicsContext.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <cstring>


// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include "platform/GL/GLHeaders.hpp"
namespace astraeus {

// Numerical precision constants for unprojection
constexpr float MATRIX_SINGULARITY_EPSILON = 1e-12f;  // For matrix inversion
constexpr float PERSPECTIVE_DIVIDE_EPSILON = 1e-12f;  // For w-coordinate guard

/**
 * OpenGL render device implementation.
 * Uses offscreen context for headless rendering with readback support.
 */
class GLRenderDevice : public RenderDevice {
public:
    explicit GLRenderDevice(const Config& config);
    ~GLRenderDevice() override;

    bool initialize() override;
    void shutdown() override;

    void begin_frame() override;
    void end_frame() override;
    void resize(uint32_t width, uint32_t height) override;

    void get_color_buffer_view(PixelBufferView& out_view) const override;
    void get_id_buffer_view(PixelBufferView& out_view) const override;
    void pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result) const override;

    // GPU resource management
    struct BufferHandle {
        uint32_t gl_id = 0;
        uint32_t size = 0;
    };

    struct TextureHandle {
        uint32_t gl_id = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t format = 0;
    };

    struct ShaderHandle {
        uint32_t gl_program = 0;
        std::unordered_map<std::string, int32_t> uniform_locations;
    };

    struct PipelineState {
        ShaderHandle* shader = nullptr;
        bool depth_test_enabled = true;
        bool blend_enabled = false;
    };

    // Resource creation/destruction
    BufferHandle create_buffer(const void* data, uint32_t size, uint32_t usage);
    void destroy_buffer(BufferHandle& handle);

    TextureHandle create_texture(uint32_t width, uint32_t height, uint32_t format, const void* data = nullptr);
    void destroy_texture(TextureHandle& handle);

    ShaderHandle create_shader(const char* vertex_src, const char* fragment_src, std::string& error_msg);
    void destroy_shader(ShaderHandle& handle);

    // Rendering commands
    void bind_shader(const ShaderHandle& shader);
    void set_uniform_mat4(const ShaderHandle& shader, const char* name, const float* matrix);
    void set_uniform_float(const ShaderHandle& shader, const char* name, float value);
    void set_uniform_vec3(const ShaderHandle& shader, const char* name, float x, float y, float z);
    
    void draw_arrays(uint32_t primitive_type, uint32_t first, uint32_t count);
    void draw_indexed(uint32_t primitive_type, uint32_t index_count, uint32_t index_type);

    // Camera matrix management for picking
    void set_view_projection_matrix(const float* view_projection);
    
    // Debug support
    void push_debug_group(const char* name);
    void pop_debug_group();
    void set_object_label(uint32_t gl_type, uint32_t gl_id, const char* label);

private:
    // Unprojection helpers
    bool invert_matrix_4x4(const float* m, float* out_inv) const;
    void unproject(float screen_x, float screen_y, float depth, 
                   const float* inv_vp, 
                   float& out_world_x, float& out_world_y, float& out_world_z) const;
    void create_offscreen_context();
    void destroy_offscreen_context();
    void create_framebuffers();
    void destroy_framebuffers();
    void create_readback_buffers();
    void destroy_readback_buffers();
    void setup_debug_output();

    // OpenGL context (platform-independent via backend)
    GraphicsContext* graphics_context_;

    // Frame timing
    std::chrono::time_point<std::chrono::high_resolution_clock> frame_start_time_;

    // Framebuffer objects
    uint32_t main_fbo_;
    uint32_t color_texture_;
    uint32_t id_texture_;
    uint32_t depth_texture_;

    // Pixel buffer objects for readback
    uint32_t color_pbo_;
    uint32_t id_pbo_;
    uint32_t depth_pbo_;             // PBO for depth readback
    void* color_mapped_ptr_;
    void* id_mapped_ptr_;
    void* depth_mapped_ptr_;         // Mapped pointer for depth buffer

    bool supports_persistent_map_ = false;
    uint32_t color_pbo_bytes_ = 0;
    uint32_t id_pbo_bytes_ = 0;
    uint32_t depth_pbo_bytes_ = 0;   // Size of depth PBO

    std::vector<std::uint8_t> color_cpu_;
    std::vector<std::uint8_t> id_cpu_;
    std::vector<std::uint8_t> depth_cpu_;  // CPU-backed depth buffer (fallback)


    
    // Fence sync objects for GPU/CPU synchronization
    void* color_fence_;
    void* id_fence_;
    void* depth_fence_;              // Fence for depth buffer sync
    
    // Camera matrices for unprojection (updated each frame from World)
    float cached_view_projection_matrix_[16];
    float cached_inv_view_projection_[16];
    bool cached_matrices_valid_ = false;

    // Debug support
    bool has_khr_debug_;
    bool debug_output_enabled_;
};

// ============================================================================
// Inline implementations
// ============================================================================

// Debug callback for OpenGL errors
static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id,
                                         GLenum severity, GLsizei length,
                                         const GLchar* message, const void* userParam) {
    (void)source;
    (void)id;
    (void)length;
    (void)userParam;
    
    // Filter out non-significant errors
    if (type == GL_DEBUG_TYPE_OTHER) return;
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

    std::cerr << "[OpenGL] ";
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: std::cerr << "HIGH "; break;
        case GL_DEBUG_SEVERITY_MEDIUM: std::cerr << "MEDIUM "; break;
        case GL_DEBUG_SEVERITY_LOW: std::cerr << "LOW "; break;
    }

    std::cerr << "severity: " << message << std::endl;
}

inline GLRenderDevice::GLRenderDevice(const Config& config)
    : RenderDevice(config)
    , graphics_context_(nullptr)
    , main_fbo_(0)
    , color_texture_(0)
    , id_texture_(0)
    , depth_texture_(0)
    , color_pbo_(0)
    , id_pbo_(0)
    , depth_pbo_(0)
    , color_mapped_ptr_(nullptr)
    , id_mapped_ptr_(nullptr)
    , depth_mapped_ptr_(nullptr)
    , color_fence_(nullptr)
    , id_fence_(nullptr)
    , depth_fence_(nullptr)
    , has_khr_debug_(false)
    , debug_output_enabled_(false)
    , cached_view_projection_matrix_{0}      // Zero-initialize
    , cached_inv_view_projection_{0}          // Zero-initialize
{
    // Set identity matrix diagonal elements
    cached_view_projection_matrix_[0] = cached_view_projection_matrix_[5] = 
        cached_view_projection_matrix_[10] = cached_view_projection_matrix_[15] = 1.0f;
    cached_inv_view_projection_[0] = cached_inv_view_projection_[5] = 
        cached_inv_view_projection_[10] = cached_inv_view_projection_[15] = 1.0f;
}

inline GLRenderDevice::~GLRenderDevice() {
    shutdown();
}

inline bool GLRenderDevice::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[GLRenderDevice] Initializing OpenGL backend " << width_ << "x" << height_ << std::endl;

    create_offscreen_context();
    if (!graphics_context_) {
        std::cerr << "[GLRenderDevice] Failed to create graphics context" << std::endl;
        return false;
    }

    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    std::cout << "[GLRenderDevice] OpenGL version: " << (version ? version : "<null>") << std::endl;
    std::cout << "[GLRenderDevice] Renderer: " << (renderer ? renderer : "<null>") << std::endl;

    if (config_.enable_debug) {
        setup_debug_output();
    }

    create_framebuffers();
    create_readback_buffers();

    stats_.draw_calls = 0;
    stats_.triangle_count = 0;
    stats_.render_time_ms = 0.0;

    is_initialized_ = true;
    std::cout << "[GLRenderDevice] Initialization complete" << std::endl;
    return true;
}

inline void GLRenderDevice::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[GLRenderDevice] Shutting down" << std::endl;

    destroy_readback_buffers();
    destroy_framebuffers();
    destroy_offscreen_context();

    is_initialized_ = false;
}

inline void GLRenderDevice::begin_frame() {
    // If fallback mapping, unmap previous mapping so the PBO can be written again
    // if (!supports_persistent_map_) {
    //     if (color_mapped_ptr_) {
    //         glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
    //         glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    //         color_mapped_ptr_ = nullptr;
    //     }
    //     if (id_mapped_ptr_) {
    //         glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
    //         glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    //         id_mapped_ptr_ = nullptr;
    //     }
    //     glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    // }

    frame_start_time_ = std::chrono::high_resolution_clock::now();
    stats_.draw_calls = 0;
    stats_.triangle_count = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, main_fbo_);
    glViewport(0, 0, width_, height_);
}


inline void GLRenderDevice::end_frame() {
    // Wait for previous frame's fence (if any)
    if (color_fence_) {
        GLenum result = glClientWaitSync(static_cast<GLsync>(color_fence_),
                                         GL_SYNC_FLUSH_COMMANDS_BIT,
                                         1000000000);
        if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
            std::cerr << "[GLRenderDevice] Color fence wait failed or timed out" << std::endl;
        }
        glDeleteSync(static_cast<GLsync>(color_fence_));
        color_fence_ = nullptr;
    }

    if (id_fence_) {
        GLenum result = glClientWaitSync(static_cast<GLsync>(id_fence_),
                                         GL_SYNC_FLUSH_COMMANDS_BIT,
                                         1000000000);
        if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
            std::cerr << "[GLRenderDevice] ID fence wait failed or timed out" << std::endl;
        }
        glDeleteSync(static_cast<GLsync>(id_fence_));
        id_fence_ = nullptr;
    }

    if (depth_fence_) {
        GLenum result = glClientWaitSync(static_cast<GLsync>(depth_fence_),
                                         GL_SYNC_FLUSH_COMMANDS_BIT,
                                         1000000000);
        if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
            std::cerr << "[GLRenderDevice] Depth fence wait failed or timed out" << std::endl;
        }
        glDeleteSync(static_cast<GLsync>(depth_fence_));
        depth_fence_ = nullptr;
    }

    // Readback: texture -> PBO
    glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
    glBindTexture(GL_TEXTURE_2D, color_texture_);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
    glBindTexture(GL_TEXTURE_2D, id_texture_);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

    // Readback depth buffer: read depth component as float
    glBindBuffer(GL_PIXEL_PACK_BUFFER, depth_pbo_);
    glBindTexture(GL_TEXTURE_2D, depth_texture_);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    // If no persistent map, copy PBO -> CPU-backed stable buffer
    if (!supports_persistent_map_) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
        glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, color_pbo_bytes_, color_cpu_.data());

        glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
        glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, id_pbo_bytes_, id_cpu_.data());

        glBindBuffer(GL_PIXEL_PACK_BUFFER, depth_pbo_);
        glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, depth_pbo_bytes_, depth_cpu_.data());
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // Fence indicates GPU commands for readback have been queued/completed
    color_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    id_fence_    = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    depth_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // Frame time
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - frame_start_time_;
    stats_.render_time_ms = elapsed.count();
}


inline void GLRenderDevice::resize(uint32_t width, uint32_t height) {
    if (width == width_ && height == height_) {
        return;
    }

    std::cout << "[GLRenderDevice] Resizing to " << width << "x" << height << std::endl;
    
    // Check if new size exceeds PBO capacity
    uint32_t color_buffer_size = width * height * 4;
    uint32_t current_pbo_size = width_ * height_ * 4;
    
    if (color_buffer_size > current_pbo_size) {
        std::cerr << "[GLRenderDevice] WARNING: Resize requires larger PBO ("
                  << color_buffer_size << " > " << current_pbo_size 
                  << "). Recreating readback buffers - pointer will change!" << std::endl;
        
        // Must recreate PBOs - pointer stability will be broken
        // This should be avoided by using max backing size from RenderDevice base
        destroy_readback_buffers();
        width_ = width;
        height_ = height;
        create_readback_buffers();
    } else {
        // Just update dimensions - PBOs remain stable
        width_ = width;
        height_ = height;
    }

    // Always recreate textures and FBO (they don't affect Java pointer stability)
    // First, clean up only textures and FBO, not PBOs
    if (main_fbo_ != 0) {
        glDeleteFramebuffers(1, &main_fbo_);
        main_fbo_ = 0;
    }
    if (color_texture_ != 0) {
        glDeleteTextures(1, &color_texture_);
        color_texture_ = 0;
    }
    if (id_texture_ != 0) {
        glDeleteTextures(1, &id_texture_);
        id_texture_ = 0;
    }
    if (depth_texture_ != 0) {
        glDeleteTextures(1, &depth_texture_);
        depth_texture_ = 0;
    }
    
    // Recreate textures and FBO with new size
    create_framebuffers();
}

/**
 * Get a view of the color buffer for readback (zero-copy).
 * 
 * STABILITY GUARANTEES:
 * - Pointer (out_view.data) is ALWAYS VALID and NON-NULL after initialization
 * - Pointer is STABLE across frames (never changes unless resize exceeds capacity)
 * - Pointer is persistently mapped using GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
 * - Java can safely read from this pointer at any time (FFM MemorySegment integration)
 * - Fence syncs ensure GPU writes complete before Java reads
 * 
 * THREAD SAFETY:
 * - Coherent mapping ensures visibility without explicit synchronization
 * - Fence objects track GPU completion for deterministic reads
 * 
 * @param out_view Output pixel buffer view with stable pointer
 */
    inline void GLRenderDevice::get_color_buffer_view(PixelBufferView& out_view) const {
    out_view.data   = color_mapped_ptr_;
    out_view.width  = width_;
    out_view.height = height_;
    out_view.stride = width_ * 4;                 // RGBA8 => 4 bytes per pixel
    out_view.format = PIXEL_FORMAT_RGBA8;         // use enum, not 0

    // Backing capacity (stable allocation size)
    // NOTE: these should reflect the allocation that backs `data`, not necessarily current viewport.
    // If you allocate exactly width_*height_*4 each time, then max==current.
    // If you allocate a larger "max backing" buffer, set these to that max capacity.
    const uint32_t bytes_per_pixel = 4;
    const uint32_t backing_bytes = color_pbo_bytes_; // or the CPU buffer size

    // If you *don’t* track max_backing_width/height separately, you can treat current as max.
    // Better: store max_backing_width_/height_ in the base RenderDevice if you have it.
    out_view.max_backing_width  = width_;
    out_view.max_backing_height = height_;
    out_view.max_backing_size   = backing_bytes * bytes_per_pixel;

    // Optional defensive clamp: ensure max_backing_size is at least current frame footprint
    // (useful if something got out of sync)
    const uint64_t needed = (uint64_t)out_view.stride * (uint64_t)out_view.height;
    if (out_view.max_backing_size < needed) {
        // keep it consistent rather than lying
        out_view.max_backing_size = (uint32_t)needed;
        out_view.max_backing_width = out_view.width;
        out_view.max_backing_height = out_view.height;
    }
}


/**
 * Get a view of the ID buffer for picking (zero-copy).
 * 
 * STABILITY GUARANTEES:
 * - Same stability guarantees as color buffer
 * - Pointer is ALWAYS VALID and NON-NULL after initialization
 * - Persistently mapped with coherent visibility
 * 
 * @param out_view Output pixel buffer view with stable pointer
 */
inline void GLRenderDevice::get_id_buffer_view(PixelBufferView& out_view) const {
    out_view.data   = id_mapped_ptr_;
    out_view.width  = width_;
    out_view.height = height_;
    out_view.stride = width_ * 4;                 // R32UI => 4 bytes per pixel
    out_view.format = PIXEL_FORMAT_R32UI;         // use enum value

    const uint32_t backing_bytes = id_pbo_bytes_;

    out_view.max_backing_width  = width_;
    out_view.max_backing_height = height_;
    out_view.max_backing_size   = backing_bytes;

    const uint64_t needed = (uint64_t)out_view.stride * (uint64_t)out_view.height;
    if (out_view.max_backing_size < needed) {
        out_view.max_backing_size = (uint32_t)needed;
        out_view.max_backing_width = out_view.width;
        out_view.max_backing_height = out_view.height;
    }
}


inline void GLRenderDevice::pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result) const {
    // Initialize result to no hit
    out_result.hit = false;
    out_result.entity_id = 0;
    out_result.depth = 1.0f;
    out_result.world_x = 0.0f;
    out_result.world_y = 0.0f;
    out_result.world_z = 0.0f;
    
    // Boundary check
    if (!id_mapped_ptr_ || !depth_mapped_ptr_ || screen_x >= width_ || screen_y >= height_) {
        return;
    }

    // Read from ID buffer (flip Y coordinate for OpenGL)
    uint32_t y_flipped = height_ - 1 - screen_y;
    uint32_t offset = y_flipped * width_ + screen_x;
    
    uint32_t* id_data = static_cast<uint32_t*>(id_mapped_ptr_);
    uint32_t entity_id = id_data[offset];

    if (entity_id == 0) {
        return;  // No entity hit
    }

    // Read depth value from depth buffer
    float* depth_data = static_cast<float*>(depth_mapped_ptr_);
    float depth = depth_data[offset];
    
    // Reconstruct world position using unprojection
    float world_x, world_y, world_z;
    if (cached_matrices_valid_) {
        unproject(static_cast<float>(screen_x), static_cast<float>(screen_y), depth,
                  cached_inv_view_projection_, world_x, world_y, world_z);
    } else {
        // No valid camera matrices, return screen-space coordinates only
        world_x = static_cast<float>(screen_x);
        world_y = static_cast<float>(screen_y);
        world_z = depth;
    }

    // Populate result
    out_result.hit = true;
    out_result.entity_id = entity_id;
    out_result.depth = depth;
    out_result.world_x = world_x;
    out_result.world_y = world_y;
    out_result.world_z = world_z;
}

// Resource management
inline GLRenderDevice::BufferHandle GLRenderDevice::create_buffer(const void* data, uint32_t size, uint32_t usage) {
    BufferHandle handle;
    handle.size = size;

    glGenBuffers(1, &handle.gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, handle.gl_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return handle;
}

inline void GLRenderDevice::destroy_buffer(BufferHandle& handle) {
    if (handle.gl_id != 0) {
        glDeleteBuffers(1, &handle.gl_id);
        handle.gl_id = 0;
        handle.size = 0;
    }
}

inline GLRenderDevice::TextureHandle GLRenderDevice::create_texture(uint32_t width, uint32_t height, 
                                                              uint32_t format, const void* data) {
    TextureHandle handle;
    handle.width = width;
    handle.height = height;
    handle.format = format;

    glGenTextures(1, &handle.gl_id);
    glBindTexture(GL_TEXTURE_2D, handle.gl_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return handle;
}

inline void GLRenderDevice::destroy_texture(TextureHandle& handle) {
    if (handle.gl_id != 0) {
        glDeleteTextures(1, &handle.gl_id);
        handle.gl_id = 0;
    }
}

inline GLRenderDevice::ShaderHandle GLRenderDevice::create_shader(const char* vertex_src, 
                                                            const char* fragment_src,
                                                            std::string& error_msg) {
    ShaderHandle handle;

    // Compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_src, nullptr);
    glCompileShader(vertex_shader);

    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        error_msg = std::string("Vertex shader compilation failed: ") + info_log;
        glDeleteShader(vertex_shader);
        return handle;
    }

    // Compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_src, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        error_msg = std::string("Fragment shader compilation failed: ") + info_log;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return handle;
    }

    // Link program
    handle.gl_program = glCreateProgram();
    glAttachShader(handle.gl_program, vertex_shader);
    glAttachShader(handle.gl_program, fragment_shader);
    glLinkProgram(handle.gl_program);

    glGetProgramiv(handle.gl_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(handle.gl_program, 512, nullptr, info_log);
        error_msg = std::string("Shader program linking failed: ") + info_log;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(handle.gl_program);
        handle.gl_program = 0;
        return handle;
    }

    // Cleanup intermediate shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return handle;
}

inline void GLRenderDevice::destroy_shader(ShaderHandle& handle) {
    if (handle.gl_program != 0) {
        glDeleteProgram(handle.gl_program);
        handle.gl_program = 0;
        handle.uniform_locations.clear();
    }
}

inline void GLRenderDevice::bind_shader(const ShaderHandle& shader) {
    glUseProgram(shader.gl_program);
}

inline void GLRenderDevice::set_uniform_mat4(const ShaderHandle& shader, const char* name, const float* matrix) {
    GLint location = glGetUniformLocation(shader.gl_program, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

inline void GLRenderDevice::set_uniform_float(const ShaderHandle& shader, const char* name, float value) {
    GLint location = glGetUniformLocation(shader.gl_program, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

inline void GLRenderDevice::set_uniform_vec3(const ShaderHandle& shader, const char* name, float x, float y, float z) {
    GLint location = glGetUniformLocation(shader.gl_program, name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

inline void GLRenderDevice::draw_arrays(uint32_t primitive_type, uint32_t first, uint32_t count) {
    glDrawArrays(primitive_type, first, count);
    stats_.draw_calls++;
    if (primitive_type == GL_TRIANGLES) {
        stats_.triangle_count += count / 3;
    }
}

inline void GLRenderDevice::draw_indexed(uint32_t primitive_type, uint32_t index_count, uint32_t index_type) {
    glDrawElements(primitive_type, index_count, index_type, nullptr);
    stats_.draw_calls++;
    if (primitive_type == GL_TRIANGLES) {
        stats_.triangle_count += index_count / 3;
    }
}

inline void GLRenderDevice::push_debug_group(const char* name) {
    if (has_khr_debug_) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
    }
}

inline void GLRenderDevice::pop_debug_group() {
    if (has_khr_debug_) {
        glPopDebugGroup();
    }
}

inline void GLRenderDevice::set_object_label(uint32_t gl_type, uint32_t gl_id, const char* label) {
    if (has_khr_debug_) {
        glObjectLabel(gl_type, gl_id, -1, label);
    }
}

// Private methods
inline void GLRenderDevice::create_offscreen_context() {
    // Use factory to create platform-appropriate context
    graphics_context_ = create_graphics_context();
    
    if (!graphics_context_) {
        std::cerr << "[GLRenderDevice] Failed to allocate graphics context" << std::endl;
        return;
    }

    std::cout << "[GLRenderDevice] Using backend: " << graphics_context_->get_backend_name() << std::endl;

    // Initialize the context
    if (!graphics_context_->initialize(width_, height_)) {
        std::cerr << "[GLRenderDevice] Failed to initialize graphics context" << std::endl;
        delete graphics_context_;
        graphics_context_ = nullptr;
        return;
    }
}

inline void GLRenderDevice::destroy_offscreen_context() {
    if (graphics_context_) {
        graphics_context_->shutdown();
        delete graphics_context_;
        graphics_context_ = nullptr;
    }
}

inline void GLRenderDevice::create_framebuffers() {
    std::cout << "[GLRenderDevice] Creating framebuffers " << width_ << "x" << height_ << std::endl;

    // Create color texture
    glGenTextures(1, &color_texture_);
    glBindTexture(GL_TEXTURE_2D, color_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    set_object_label(GL_TEXTURE, color_texture_, "ColorTexture");

    // Create ID texture for picking
    glGenTextures(1, &id_texture_);
    glBindTexture(GL_TEXTURE_2D, id_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width_, height_, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    set_object_label(GL_TEXTURE, id_texture_, "IDTexture");

    // Create depth texture
    glGenTextures(1, &depth_texture_);
    glBindTexture(GL_TEXTURE_2D, depth_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    set_object_label(GL_TEXTURE, depth_texture_, "DepthTexture");

    // Create framebuffer
    glGenFramebuffers(1, &main_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, main_fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, id_texture_, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_texture_, 0);
    set_object_label(GL_FRAMEBUFFER, main_fbo_, "MainFBO");

    // Set draw buffers
    GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, draw_buffers);

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[GLRenderDevice] Framebuffer is not complete: " << status << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "[GLRenderDevice] Framebuffers created successfully" << std::endl;
}

inline void GLRenderDevice::create_readback_buffers() {
    std::cout << "[GLRenderDevice] Creating readback buffers " << width_ << "x" << height_ << std::endl;

    color_pbo_bytes_ = width_ * height_ * 4; // RGBA8
    id_pbo_bytes_    = width_ * height_ * 4; // R32UI

    // Detect persistent mapping support (GL 4.4+ or ARB_buffer_storage) and function pointer presence.
    supports_persistent_map_ = false;

    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    const bool gl44plus = (major > 4) || (major == 4 && minor >= 4);

    bool has_arb_buffer_storage = false;
    GLint num_ext = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
    for (GLint i = 0; i < num_ext; ++i) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (ext && std::strcmp(ext, "GL_ARB_buffer_storage") == 0) {
            has_arb_buffer_storage = true;
            break;
        }
    }

    if ((gl44plus || has_arb_buffer_storage) && glBufferStorage) {
        supports_persistent_map_ = true;
    }

    std::cout << "[GLRenderDevice] Persistent map: "
              << (supports_persistent_map_ ? "ENABLED" : "DISABLED")
              << " (GL " << major << "." << minor
              << ", ARB_buffer_storage=" << (has_arb_buffer_storage ? "YES" : "NO")
              << ", glBufferStorage=" << (glBufferStorage ? "OK" : "NULL")
              << ")\n";

    // ---------------------------------------------------------------------
    // Color PBO
    // ---------------------------------------------------------------------
    glGenBuffers(1, &color_pbo_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);

    if (supports_persistent_map_) {
        const GLbitfield storage_flags = GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_PIXEL_PACK_BUFFER, color_pbo_bytes_, nullptr, storage_flags);

        const GLbitfield map_flags = GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        color_mapped_ptr_ = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, color_pbo_bytes_, map_flags);

        if (!color_mapped_ptr_) {
            std::cerr << "[GLRenderDevice] Failed to persistently map color PBO" << std::endl;
        }
    } else {
        // GL 3.3 fallback: PBO exists but exported pointer is CPU-backed and stable.
        glBufferData(GL_PIXEL_PACK_BUFFER, color_pbo_bytes_, nullptr, GL_STREAM_READ);

        color_cpu_.assign(color_pbo_bytes_, 0);
        color_mapped_ptr_ = color_cpu_.data(); // stable, non-null after init
    }

    set_object_label(GL_BUFFER, color_pbo_, "ColorPBO");

    // ---------------------------------------------------------------------
    // ID PBO
    // ---------------------------------------------------------------------
    glGenBuffers(1, &id_pbo_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);

    if (supports_persistent_map_) {
        const GLbitfield storage_flags = GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_PIXEL_PACK_BUFFER, id_pbo_bytes_, nullptr, storage_flags);

        const GLbitfield map_flags = GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        id_mapped_ptr_ = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, id_pbo_bytes_, map_flags);

        if (!id_mapped_ptr_) {
            std::cerr << "[GLRenderDevice] Failed to persistently map ID PBO" << std::endl;
        }
    } else {
        glBufferData(GL_PIXEL_PACK_BUFFER, id_pbo_bytes_, nullptr, GL_STREAM_READ);

        id_cpu_.assign(id_pbo_bytes_, 0);
        id_mapped_ptr_ = id_cpu_.data(); // stable, non-null after init
    }

    set_object_label(GL_BUFFER, id_pbo_, "IDPBO");

    // ---------------------------------------------------------------------
    // Depth PBO
    // ---------------------------------------------------------------------
    depth_pbo_bytes_ = width_ * height_ * 4; // Depth as GL_FLOAT (4 bytes)
    
    glGenBuffers(1, &depth_pbo_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, depth_pbo_);

    if (supports_persistent_map_) {
        const GLbitfield storage_flags = GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_PIXEL_PACK_BUFFER, depth_pbo_bytes_, nullptr, storage_flags);

        const GLbitfield map_flags = GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        depth_mapped_ptr_ = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, depth_pbo_bytes_, map_flags);

        if (!depth_mapped_ptr_) {
            std::cerr << "[GLRenderDevice] Failed to persistently map depth PBO" << std::endl;
        }
    } else {
        glBufferData(GL_PIXEL_PACK_BUFFER, depth_pbo_bytes_, nullptr, GL_STREAM_READ);

        depth_cpu_.assign(depth_pbo_bytes_, 0);
        depth_mapped_ptr_ = depth_cpu_.data(); // stable, non-null after init
    }

    set_object_label(GL_BUFFER, depth_pbo_, "DepthPBO");

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    std::cout << "[GLRenderDevice] Readback buffers created successfully" << std::endl;
}



inline void GLRenderDevice::destroy_framebuffers() {
    // Only destroy textures and FBO, not PBOs
    if (main_fbo_ != 0) {
        glDeleteFramebuffers(1, &main_fbo_);
        main_fbo_ = 0;
    }

    if (color_texture_ != 0) {
        glDeleteTextures(1, &color_texture_);
        color_texture_ = 0;
    }

    if (id_texture_ != 0) {
        glDeleteTextures(1, &id_texture_);
        id_texture_ = 0;
    }

    if (depth_texture_ != 0) {
        glDeleteTextures(1, &depth_texture_);
        depth_texture_ = 0;
    }
}

    inline void GLRenderDevice::destroy_readback_buffers() {
    if (color_fence_) {
        glDeleteSync(static_cast<GLsync>(color_fence_));
        color_fence_ = nullptr;
    }
    if (id_fence_) {
        glDeleteSync(static_cast<GLsync>(id_fence_));
        id_fence_ = nullptr;
    }
    if (depth_fence_) {
        glDeleteSync(static_cast<GLsync>(depth_fence_));
        depth_fence_ = nullptr;
    }

    // If persistently mapped, unmap before delete
    if (supports_persistent_map_) {
        if (color_pbo_ != 0 && color_mapped_ptr_) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            color_mapped_ptr_ = nullptr;
        }
        if (id_pbo_ != 0 && id_mapped_ptr_) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            id_mapped_ptr_ = nullptr;
        }
        if (depth_pbo_ != 0 && depth_mapped_ptr_) {
            glBindBuffer(GL_PIXEL_PACK_BUFFER, depth_pbo_);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            depth_mapped_ptr_ = nullptr;
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    } else {
        // CPU-backed fallback pointers
        color_mapped_ptr_ = nullptr;
        id_mapped_ptr_ = nullptr;
        depth_mapped_ptr_ = nullptr;
        color_cpu_.clear();
        id_cpu_.clear();
        depth_cpu_.clear();
    }

    if (color_pbo_ != 0) {
        glDeleteBuffers(1, &color_pbo_);
        color_pbo_ = 0;
    }
    if (id_pbo_ != 0) {
        glDeleteBuffers(1, &id_pbo_);
        id_pbo_ = 0;
    }
    if (depth_pbo_ != 0) {
        glDeleteBuffers(1, &depth_pbo_);
        depth_pbo_ = 0;
    }

    supports_persistent_map_ = false;
    color_pbo_bytes_ = 0;
    id_pbo_bytes_ = 0;
    depth_pbo_bytes_ = 0;
}


inline void GLRenderDevice::setup_debug_output() {
    // Check for KHR_debug extension
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    
    for (GLint i = 0; i < num_extensions; i++) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (strcmp(ext, "GL_KHR_debug") == 0) {
            has_khr_debug_ = true;
            break;
        }
    }

    if (has_khr_debug_) {
        std::cout << "[GLRenderDevice] Enabling KHR_debug support" << std::endl;
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        debug_output_enabled_ = true;
    } else {
        std::cout << "[GLRenderDevice] KHR_debug not available" << std::endl;
    }
}

// =============================================================================
// Camera matrix management and unprojection
// =============================================================================

inline void GLRenderDevice::set_view_projection_matrix(const float* view_projection) {
    if (!view_projection) {
        cached_matrices_valid_ = false;
        return;
    }
    
    // Copy the view-projection matrix
    std::memcpy(cached_view_projection_matrix_, view_projection, 16 * sizeof(float));
    
    // Compute inverse for unprojection
    cached_matrices_valid_ = invert_matrix_4x4(cached_view_projection_matrix_, 
                                                cached_inv_view_projection_);
}

/**
 * Invert a 4x4 matrix using Gaussian elimination.
 * Returns true if successful, false if matrix is singular.
 */
inline bool GLRenderDevice::invert_matrix_4x4(const float* m, float* out_inv) const {
    // Create an augmented matrix [M | I]
    float aug[4][8];
    
    // Initialize augmented matrix
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            aug[row][col] = m[col * 4 + row];  // Column-major to row-major
            aug[row][col + 4] = (row == col) ? 1.0f : 0.0f;  // Identity on right
        }
    }
    
    // Gaussian elimination with partial pivoting
    for (int col = 0; col < 4; ++col) {
        // Find pivot
        int pivot_row = col;
        float max_val = math::abs(aug[col][col]);
        for (int row = col + 1; row < 4; ++row) {
            float val = math::abs(aug[row][col]);
            if (val > max_val) {
                max_val = val;
                pivot_row = row;
            }
        }
        
        // Check for singularity
        if (max_val < MATRIX_SINGULARITY_EPSILON) {
            return false;  // Matrix is singular
        }
        
        // Swap rows if needed
        if (pivot_row != col) {
            for (int k = 0; k < 8; ++k) {
                float temp = aug[col][k];
                aug[col][k] = aug[pivot_row][k];
                aug[pivot_row][k] = temp;
            }
        }
        
        // Scale pivot row
        float pivot = aug[col][col];
        for (int k = 0; k < 8; ++k) {
            aug[col][k] /= pivot;
        }
        
        // Eliminate column
        for (int row = 0; row < 4; ++row) {
            if (row != col) {
                float factor = aug[row][col];
                for (int k = 0; k < 8; ++k) {
                    aug[row][k] -= factor * aug[col][k];
                }
            }
        }
    }
    
    // Extract inverse from right half (convert back to column-major)
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            out_inv[col * 4 + row] = aug[row][col + 4];
        }
    }
    
    return true;
}

/**
 * Unproject screen coordinates + depth to world space.
 * 
 * @param screen_x Screen X coordinate (0 to width-1)
 * @param screen_y Screen Y coordinate (0 to height-1) 
 * @param depth Depth value from depth buffer [0, 1]
 * @param inv_vp Inverse view-projection matrix
 * @param out_world_x Output world X coordinate
 * @param out_world_y Output world Y coordinate
 * @param out_world_z Output world Z coordinate
 */
inline void GLRenderDevice::unproject(float screen_x, float screen_y, float depth,
                                      const float* inv_vp,
                                      float& out_world_x, float& out_world_y, float& out_world_z) const {
    // Convert screen coordinates to normalized device coordinates (NDC)
    // Screen space: (0, 0) at top-left, (width, height) at bottom-right
    // NDC space: (-1, -1) at bottom-left, (1, 1) at top-right
    // Depth: [0, 1] in OpenGL depth buffer maps to [-1, 1] in NDC
    
    float ndc_x = (2.0f * screen_x) / width_ - 1.0f;
    float ndc_y = 1.0f - (2.0f * screen_y) / height_;  // Flip Y
    float ndc_z = 2.0f * depth - 1.0f;  // Map [0,1] to [-1,1]
    
    // Homogeneous clip coordinates
    float clip_x = ndc_x;
    float clip_y = ndc_y;
    float clip_z = ndc_z;
    float clip_w = 1.0f;
    
    // Transform by inverse view-projection matrix
    float world_x = inv_vp[0] * clip_x + inv_vp[4] * clip_y + inv_vp[8] * clip_z + inv_vp[12] * clip_w;
    float world_y = inv_vp[1] * clip_x + inv_vp[5] * clip_y + inv_vp[9] * clip_z + inv_vp[13] * clip_w;
    float world_z = inv_vp[2] * clip_x + inv_vp[6] * clip_y + inv_vp[10] * clip_z + inv_vp[14] * clip_w;
    float world_w = inv_vp[3] * clip_x + inv_vp[7] * clip_y + inv_vp[11] * clip_z + inv_vp[15] * clip_w;
    
    // Perspective divide
    if (math::abs(world_w) > PERSPECTIVE_DIVIDE_EPSILON) {
        out_world_x = world_x / world_w;
        out_world_y = world_y / world_w;
        out_world_z = world_z / world_w;
    } else {
        // Fallback for degenerate case
        out_world_x = world_x;
        out_world_y = world_y;
        out_world_z = world_z;
    }
}

} // namespace astraeus

#endif // ASTRAEUS_GL_RENDER_DEVICE_HPP
