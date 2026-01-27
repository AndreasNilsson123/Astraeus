#ifndef ASTRAEUS_GL_RENDER_DEVICE_HPP
#define ASTRAEUS_GL_RENDER_DEVICE_HPP

#include "../RenderDevice.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>

namespace astraeus {

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

    // Debug support
    void push_debug_group(const char* name);
    void pop_debug_group();
    void set_object_label(uint32_t gl_type, uint32_t gl_id, const char* label);

private:
    void create_offscreen_context();
    void destroy_offscreen_context();
    void create_framebuffers();
    void destroy_framebuffers();
    void setup_debug_output();

    // OpenGL context (platform-specific)
    void* gl_context_;
    void* gl_display_;

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
    void* color_mapped_ptr_;
    void* id_mapped_ptr_;

    // Debug support
    bool has_khr_debug_;
    bool debug_output_enabled_;
};

} // namespace astraeus

#endif // ASTRAEUS_GL_RENDER_DEVICE_HPP
