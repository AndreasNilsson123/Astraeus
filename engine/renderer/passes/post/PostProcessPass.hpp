#ifndef ASTRAEUS_POST_PROCESS_PASS_HPP
#define ASTRAEUS_POST_PROCESS_PASS_HPP

#include "../../RenderGraph.hpp"
#include "../../opengl/GLRenderDevice.hpp"

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

/**
 * Base class for post-processing passes.
 * Post-process passes operate on full-screen quads and process framebuffer textures.
 */
class PostProcessPass : public RenderPass {
public:
    inline PostProcessPass();
    inline ~PostProcessPass() override;

    inline bool initialize(RenderDevice* device) override;
    inline void execute(RenderDevice* device, World* world) override;
    inline void on_resize(uint32_t width, uint32_t height) override;
    
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
    inline void create_fullscreen_quad();
    
    /**
     * Destroy the full-screen quad VAO.
     */
    inline void destroy_fullscreen_quad();
    
    /**
     * Draw the full-screen quad.
     */
    inline void draw_fullscreen_quad();
    
    GLRenderDevice* gl_device_;
    uint32_t fullscreen_vao_;
    uint32_t fullscreen_vbo_;
    bool enabled_;
    uint32_t viewport_width_;
    uint32_t viewport_height_;
};

// ============================================================================
// Inline implementations
// ============================================================================

inline PostProcessPass::PostProcessPass()
    : gl_device_(nullptr)
    , fullscreen_vao_(0)
    , fullscreen_vbo_(0)
    , enabled_(true)
    , viewport_width_(0)
    , viewport_height_(0)
{
}

inline PostProcessPass::~PostProcessPass() {
    destroy_fullscreen_quad();
}

inline bool PostProcessPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }
    
    create_fullscreen_quad();
    return true;
}

inline void PostProcessPass::execute(RenderDevice* device, World* world) {
    (void)device;
    (void)world;
    // Default implementation does nothing
    // Derived classes should override apply() instead
}

inline void PostProcessPass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

inline void PostProcessPass::create_fullscreen_quad() {
    // Full-screen quad vertices (position + texcoord)
    float quad_vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &fullscreen_vao_);
    glGenBuffers(1, &fullscreen_vbo_);
    
    glBindVertexArray(fullscreen_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, fullscreen_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

inline void PostProcessPass::destroy_fullscreen_quad() {
    if (fullscreen_vao_ != 0) {
        glDeleteVertexArrays(1, &fullscreen_vao_);
        fullscreen_vao_ = 0;
    }
    if (fullscreen_vbo_ != 0) {
        glDeleteBuffers(1, &fullscreen_vbo_);
        fullscreen_vbo_ = 0;
    }
}

inline void PostProcessPass::draw_fullscreen_quad() {
    glBindVertexArray(fullscreen_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

} // namespace astraeus

#endif // ASTRAEUS_POST_PROCESS_PASS_HPP
