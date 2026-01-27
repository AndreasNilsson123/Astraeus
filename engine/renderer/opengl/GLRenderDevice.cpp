#include "GLRenderDevice.hpp"
#include <iostream>
#include <cstring>
#include <chrono>

// OpenGL headers
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace astraeus {

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

GLRenderDevice::GLRenderDevice(const Config& config)
    : RenderDevice(config)
    , gl_context_(nullptr)
    , gl_display_(nullptr)
    , main_fbo_(0)
    , color_texture_(0)
    , id_texture_(0)
    , depth_texture_(0)
    , color_pbo_(0)
    , id_pbo_(0)
    , color_mapped_ptr_(nullptr)
    , id_mapped_ptr_(nullptr)
    , has_khr_debug_(false)
    , debug_output_enabled_(false)
{
}

GLRenderDevice::~GLRenderDevice() {
    shutdown();
}

bool GLRenderDevice::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[GLRenderDevice] Initializing OpenGL backend " << width_ << "x" << height_ << std::endl;

    // Create EGL offscreen context
    create_offscreen_context();
    if (!gl_context_) {
        std::cerr << "[GLRenderDevice] Failed to create OpenGL context" << std::endl;
        return false;
    }

    // Check OpenGL version
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    std::cout << "[GLRenderDevice] OpenGL version: " << version << std::endl;
    std::cout << "[GLRenderDevice] Renderer: " << renderer << std::endl;

    // Setup debug output if available
    if (config_.enable_debug) {
        setup_debug_output();
    }

    // Create framebuffers for offscreen rendering
    create_framebuffers();

    // Initialize stats
    stats_.draw_calls = 0;
    stats_.triangle_count = 0;
    stats_.render_time_ms = 0.0;

    is_initialized_ = true;
    std::cout << "[GLRenderDevice] Initialization complete" << std::endl;
    return true;
}

void GLRenderDevice::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[GLRenderDevice] Shutting down" << std::endl;

    destroy_framebuffers();
    destroy_offscreen_context();

    is_initialized_ = false;
}

void GLRenderDevice::begin_frame() {
    frame_start_time_ = std::chrono::high_resolution_clock::now();
    
    stats_.draw_calls = 0;
    stats_.triangle_count = 0;

    // Bind main framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, main_fbo_);
    glViewport(0, 0, width_, height_);
}

void GLRenderDevice::end_frame() {
    // Unmap PBOs before readback
    if (color_pbo_ != 0) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
        if (color_mapped_ptr_) {
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            color_mapped_ptr_ = nullptr;
        }
    }
    
    if (id_pbo_ != 0) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
        if (id_mapped_ptr_) {
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            id_mapped_ptr_ = nullptr;
        }
    }

    // Readback to PBOs for zero-copy access from Java
    // Bind color texture readback
    glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
    glBindTexture(GL_TEXTURE_2D, color_texture_);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Bind ID texture readback
    glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
    glBindTexture(GL_TEXTURE_2D, id_texture_);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Finish GPU work
    glFinish();

    // Remap PBOs for reading
    glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
    color_mapped_ptr_ = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    
    glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
    id_mapped_ptr_ = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // Calculate frame time
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - frame_start_time_;
    stats_.render_time_ms = elapsed.count();
}

void GLRenderDevice::resize(uint32_t width, uint32_t height) {
    if (width == width_ && height == height_) {
        return;
    }

    std::cout << "[GLRenderDevice] Resizing to " << width << "x" << height << std::endl;
    width_ = width;
    height_ = height;

    // Recreate framebuffers with new size
    destroy_framebuffers();
    create_framebuffers();
}

void GLRenderDevice::get_color_buffer_view(PixelBufferView& out_view) const {
    out_view.data = color_mapped_ptr_;
    out_view.width = width_;
    out_view.height = height_;
    out_view.stride = width_ * 4; // RGBA8
    out_view.format = 0; // RGBA8
}

void GLRenderDevice::get_id_buffer_view(PixelBufferView& out_view) const {
    out_view.data = id_mapped_ptr_;
    out_view.width = width_;
    out_view.height = height_;
    out_view.stride = width_ * 4; // R32UI
    out_view.format = 2; // R32UI
}

void GLRenderDevice::pick(uint32_t screen_x, uint32_t screen_y, PickResult& out_result) const {
    if (!id_mapped_ptr_ || screen_x >= width_ || screen_y >= height_) {
        out_result.hit = false;
        return;
    }

    // Read from ID buffer (flip Y coordinate)
    uint32_t y_flipped = height_ - 1 - screen_y;
    uint32_t offset = y_flipped * width_ + screen_x;
    uint32_t* id_data = static_cast<uint32_t*>(id_mapped_ptr_);
    uint32_t entity_id = id_data[offset];

    if (entity_id == 0) {
        out_result.hit = false;
        return;
    }

    out_result.hit = true;
    out_result.entity_id = entity_id;
    out_result.depth = 0.0f; // TODO: Read from depth buffer if needed
    out_result.world_x = 0.0f;
    out_result.world_y = 0.0f;
    out_result.world_z = 0.0f;
}

// Resource management
GLRenderDevice::BufferHandle GLRenderDevice::create_buffer(const void* data, uint32_t size, uint32_t usage) {
    BufferHandle handle;
    handle.size = size;

    glGenBuffers(1, &handle.gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, handle.gl_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return handle;
}

void GLRenderDevice::destroy_buffer(BufferHandle& handle) {
    if (handle.gl_id != 0) {
        glDeleteBuffers(1, &handle.gl_id);
        handle.gl_id = 0;
        handle.size = 0;
    }
}

GLRenderDevice::TextureHandle GLRenderDevice::create_texture(uint32_t width, uint32_t height, 
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

void GLRenderDevice::destroy_texture(TextureHandle& handle) {
    if (handle.gl_id != 0) {
        glDeleteTextures(1, &handle.gl_id);
        handle.gl_id = 0;
    }
}

GLRenderDevice::ShaderHandle GLRenderDevice::create_shader(const char* vertex_src, 
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

void GLRenderDevice::destroy_shader(ShaderHandle& handle) {
    if (handle.gl_program != 0) {
        glDeleteProgram(handle.gl_program);
        handle.gl_program = 0;
        handle.uniform_locations.clear();
    }
}

void GLRenderDevice::bind_shader(const ShaderHandle& shader) {
    glUseProgram(shader.gl_program);
}

void GLRenderDevice::set_uniform_mat4(const ShaderHandle& shader, const char* name, const float* matrix) {
    GLint location = glGetUniformLocation(shader.gl_program, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

void GLRenderDevice::set_uniform_float(const ShaderHandle& shader, const char* name, float value) {
    GLint location = glGetUniformLocation(shader.gl_program, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void GLRenderDevice::set_uniform_vec3(const ShaderHandle& shader, const char* name, float x, float y, float z) {
    GLint location = glGetUniformLocation(shader.gl_program, name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void GLRenderDevice::draw_arrays(uint32_t primitive_type, uint32_t first, uint32_t count) {
    glDrawArrays(primitive_type, first, count);
    stats_.draw_calls++;
    if (primitive_type == GL_TRIANGLES) {
        stats_.triangle_count += count / 3;
    }
}

void GLRenderDevice::draw_indexed(uint32_t primitive_type, uint32_t index_count, uint32_t index_type) {
    glDrawElements(primitive_type, index_count, index_type, nullptr);
    stats_.draw_calls++;
    if (primitive_type == GL_TRIANGLES) {
        stats_.triangle_count += index_count / 3;
    }
}

void GLRenderDevice::push_debug_group(const char* name) {
    if (has_khr_debug_) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
    }
}

void GLRenderDevice::pop_debug_group() {
    if (has_khr_debug_) {
        glPopDebugGroup();
    }
}

void GLRenderDevice::set_object_label(uint32_t gl_type, uint32_t gl_id, const char* label) {
    if (has_khr_debug_) {
        glObjectLabel(gl_type, gl_id, -1, label);
    }
}

// Private methods
void GLRenderDevice::create_offscreen_context() {
    // Use surfaceless platform for headless rendering
    const char* extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    bool has_platform_base = extensions && strstr(extensions, "EGL_EXT_platform_base");
    bool has_surfaceless = extensions && strstr(extensions, "EGL_MESA_platform_surfaceless");

    EGLDisplay display = EGL_NO_DISPLAY;
    
    if (has_platform_base && has_surfaceless) {
        // Use surfaceless platform (best for headless)
        auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
            eglGetProcAddress("eglGetPlatformDisplayEXT");
        if (eglGetPlatformDisplayEXT) {
            display = eglGetPlatformDisplayEXT(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
        }
    }
    
    if (display == EGL_NO_DISPLAY) {
        // Fallback to default display
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }
    
    if (display == EGL_NO_DISPLAY) {
        std::cerr << "[GLRenderDevice] Failed to get EGL display" << std::endl;
        return;
    }
    
    gl_display_ = display;

    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(static_cast<EGLDisplay>(gl_display_), &major, &minor)) {
        std::cerr << "[GLRenderDevice] Failed to initialize EGL: " << eglGetError() << std::endl;
        return;
    }

    std::cout << "[GLRenderDevice] EGL version: " << major << "." << minor << std::endl;

    // Choose config
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(static_cast<EGLDisplay>(gl_display_), config_attribs, &config, 1, &num_configs)) {
        std::cerr << "[GLRenderDevice] Failed to choose EGL config" << std::endl;
        return;
    }

    // Bind OpenGL API
    eglBindAPI(EGL_OPENGL_API);

    // Create context
    EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    EGLContext context = eglCreateContext(static_cast<EGLDisplay>(gl_display_), config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "[GLRenderDevice] Failed to create EGL context" << std::endl;
        return;
    }

    // Create pbuffer surface
    EGLint pbuffer_attribs[] = {
        EGL_WIDTH, static_cast<EGLint>(width_),
        EGL_HEIGHT, static_cast<EGLint>(height_),
        EGL_NONE
    };

    EGLSurface surface = eglCreatePbufferSurface(static_cast<EGLDisplay>(gl_display_), config, pbuffer_attribs);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "[GLRenderDevice] Failed to create pbuffer surface" << std::endl;
        eglDestroyContext(static_cast<EGLDisplay>(gl_display_), context);
        return;
    }

    // Make context current
    if (!eglMakeCurrent(static_cast<EGLDisplay>(gl_display_), surface, surface, context)) {
        std::cerr << "[GLRenderDevice] Failed to make EGL context current" << std::endl;
        eglDestroySurface(static_cast<EGLDisplay>(gl_display_), surface);
        eglDestroyContext(static_cast<EGLDisplay>(gl_display_), context);
        return;
    }

    gl_context_ = context;
}

void GLRenderDevice::destroy_offscreen_context() {
    if (gl_context_) {
        eglMakeCurrent(static_cast<EGLDisplay>(gl_display_), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(static_cast<EGLDisplay>(gl_display_), static_cast<EGLContext>(gl_context_));
        gl_context_ = nullptr;
    }

    if (gl_display_) {
        eglTerminate(static_cast<EGLDisplay>(gl_display_));
        gl_display_ = nullptr;
    }
}

void GLRenderDevice::create_framebuffers() {
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

    // Create PBOs for readback (will be mapped after first frame)
    uint32_t color_buffer_size = width_ * height_ * 4; // RGBA8
    uint32_t id_buffer_size = width_ * height_ * 4; // R32UI

    glGenBuffers(1, &color_pbo_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
    glBufferData(GL_PIXEL_PACK_BUFFER, color_buffer_size, nullptr, GL_STREAM_READ);
    set_object_label(GL_BUFFER, color_pbo_, "ColorPBO");

    glGenBuffers(1, &id_pbo_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
    glBufferData(GL_PIXEL_PACK_BUFFER, id_buffer_size, nullptr, GL_STREAM_READ);
    set_object_label(GL_BUFFER, id_pbo_, "IDPBO");

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    std::cout << "[GLRenderDevice] Framebuffers created successfully" << std::endl;
}

void GLRenderDevice::destroy_framebuffers() {
    if (color_pbo_ != 0) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, color_pbo_);
        if (color_mapped_ptr_) {
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            color_mapped_ptr_ = nullptr;
        }
        glDeleteBuffers(1, &color_pbo_);
        color_pbo_ = 0;
    }

    if (id_pbo_ != 0) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, id_pbo_);
        if (id_mapped_ptr_) {
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            id_mapped_ptr_ = nullptr;
        }
        glDeleteBuffers(1, &id_pbo_);
        id_pbo_ = 0;
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

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

void GLRenderDevice::setup_debug_output() {
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

} // namespace astraeus
