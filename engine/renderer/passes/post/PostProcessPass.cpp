#include "PostProcessPass.hpp"
#include "../../opengl/GLRenderDevice.hpp"

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include "platform/GL/GLHeaders.hpp"

namespace astraeus {

PostProcessPass::PostProcessPass()
    : gl_device_(nullptr)
    , fullscreen_vao_(0)
    , fullscreen_vbo_(0)
    , enabled_(true)
    , viewport_width_(0)
    , viewport_height_(0)
{
}

PostProcessPass::~PostProcessPass() {
    destroy_fullscreen_quad();
}

bool PostProcessPass::initialize(RenderDevice* device) {
    gl_device_ = dynamic_cast<GLRenderDevice*>(device);
    if (!gl_device_) {
        return false;
    }
    
    create_fullscreen_quad();
    return true;
}

void PostProcessPass::execute(RenderDevice* device, World* world) {
    (void)device;
    (void)world;
    // Default implementation does nothing
    // Derived classes should override apply() instead
}

void PostProcessPass::on_resize(uint32_t width, uint32_t height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

void PostProcessPass::create_fullscreen_quad() {
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

void PostProcessPass::destroy_fullscreen_quad() {
    if (fullscreen_vao_ != 0) {
        glDeleteVertexArrays(1, &fullscreen_vao_);
        fullscreen_vao_ = 0;
    }
    if (fullscreen_vbo_ != 0) {
        glDeleteBuffers(1, &fullscreen_vbo_);
        fullscreen_vbo_ = 0;
    }
}

void PostProcessPass::draw_fullscreen_quad() {
    glBindVertexArray(fullscreen_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

} // namespace astraeus
