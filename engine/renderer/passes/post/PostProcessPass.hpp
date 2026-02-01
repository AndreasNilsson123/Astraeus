#ifndef ASTRAEUS_POST_PROCESS_PASS_HPP
#define ASTRAEUS_POST_PROCESS_PASS_HPP

#include <cstdint>

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

// Forward declarations
class RenderDevice;
class GLRenderDevice;
class World;

/**
 * Base class for post-processing passes.
 * Post-process passes operate on full-screen quads and process framebuffer textures.
 */
class PostProcessPass {
public:
    PostProcessPass();
    virtual ~PostProcessPass();

    virtual bool initialize(RenderDevice* device);
    virtual void execute(RenderDevice* device, World* world);
    virtual void on_resize(uint32_t width, uint32_t height);
    
    /**
     * Get a human-readable name for this pass (for telemetry)
     */
    virtual const char* get_name() const = 0;
    
    /**
     * Apply the post-processing effect.
     * @param input_texture The input texture to process
     * @param output_fbo The framebuffer to render to (0 for default)
     */
    virtual void apply(uint32_t input_texture, uint32_t output_fbo) = 0;
    
    /**
     * Get whether this pass is currently enabled.
     */
    bool is_enabled() const { return enabled_; }
    
    /**
     * Enable or disable this pass.
     */
    void set_enabled(bool enabled) { enabled_ = enabled; }

protected:
    /**
     * Create a full-screen quad VAO for post-processing.
     */
    void create_fullscreen_quad();
    
    /**
     * Destroy the full-screen quad VAO.
     */
    void destroy_fullscreen_quad();
    
    /**
     * Draw the full-screen quad.
     */
    void draw_fullscreen_quad();
    
    GLRenderDevice* gl_device_;
    uint32_t fullscreen_vao_;
    uint32_t fullscreen_vbo_;
    bool enabled_;
    uint32_t viewport_width_;
    uint32_t viewport_height_;
};

} // namespace astraeus

#endif // ASTRAEUS_POST_PROCESS_PASS_HPP
