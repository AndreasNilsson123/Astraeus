#ifndef ASTRAEUS_UNLIT_SHADER_HPP
#define ASTRAEUS_UNLIT_SHADER_HPP

#include "platform/GL/GLHeaders.hpp"#include <glad/glad.h>
#include <iostream>
#include <string>

namespace astraeus {

/**
 * UnlitShader - Simple shader for rendering meshes with vertex colors or flat color.
 */
class UnlitShader {
public:
    inline UnlitShader() : program_(0), is_compiled_(false) {}
    
    inline ~UnlitShader() {
        cleanup();
    }

    inline bool compile() {
        if (is_compiled_) {
            return true;
        }

        // Vertex shader source
        const char* vertex_source = R"(
            #version 330 core
            layout (location = 0) in vec3 aPosition;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoord;
            
            uniform mat4 uModelViewProjection;
            uniform vec4 uColor;
            
            out vec3 vNormal;
            out vec4 vColor;
            
            void main() {
                gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
                vNormal = aNormal;
                vColor = uColor;
            }
        )";

        // Fragment shader source
        const char* fragment_source = R"(
            #version 330 core
            in vec3 vNormal;
            in vec4 vColor;
            
            out vec4 FragColor;
            
            void main() {
                // Simple diffuse lighting using normal
                vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
                float diffuse = max(dot(normalize(vNormal), lightDir), 0.3);
                FragColor = vec4(vColor.rgb * diffuse, vColor.a);
            }
        )";

        // Create and compile vertex shader
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
        glCompileShader(vertex_shader);

        if (!check_shader_compile(vertex_shader, "Vertex")) {
            glDeleteShader(vertex_shader);
            return false;
        }

        // Create and compile fragment shader
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
        glCompileShader(fragment_shader);

        if (!check_shader_compile(fragment_shader, "Fragment")) {
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            return false;
        }

        // Link program
        program_ = glCreateProgram();
        glAttachShader(program_, vertex_shader);
        glAttachShader(program_, fragment_shader);
        glLinkProgram(program_);

        if (!check_program_link(program_)) {
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            glDeleteProgram(program_);
            program_ = 0;
            return false;
        }

        // Clean up shaders (no longer needed after linking)
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        // Get uniform locations
        mvp_location_ = glGetUniformLocation(program_, "uModelViewProjection");
        color_location_ = glGetUniformLocation(program_, "uColor");

        is_compiled_ = true;
        std::cout << "[UnlitShader] Shader compiled successfully" << std::endl;
        return true;
    }

    inline void use() const {
        if (is_compiled_) {
            glUseProgram(program_);
        }
    }

    inline void set_mvp(const float* matrix) const {
        if (is_compiled_ && mvp_location_ >= 0) {
            glUniformMatrix4fv(mvp_location_, 1, GL_FALSE, matrix);
        }
    }

    inline void set_color(float r, float g, float b, float a = 1.0f) const {
        if (is_compiled_ && color_location_ >= 0) {
            glUniform4f(color_location_, r, g, b, a);
        }
    }

    inline GLuint get_program() const { return program_; }
    inline bool is_compiled() const { return is_compiled_; }

private:
    inline void cleanup() {
        if (program_ != 0) {
            glDeleteProgram(program_);
            program_ = 0;
        }
        is_compiled_ = false;
    }

    inline bool check_shader_compile(GLuint shader, const char* type) {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar info_log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, info_log);
            std::cerr << "[UnlitShader] " << type << " shader compilation failed:\n" 
                      << info_log << std::endl;
            return false;
        }
        return true;
    }

    inline bool check_program_link(GLuint program) {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar info_log[1024];
            glGetProgramInfoLog(program, 1024, nullptr, info_log);
            std::cerr << "[UnlitShader] Shader program linking failed:\n" 
                      << info_log << std::endl;
            return false;
        }
        return true;
    }

    GLuint program_;
    GLint mvp_location_;
    GLint color_location_;
    bool is_compiled_;
};

} // namespace astraeus

#endif // ASTRAEUS_UNLIT_SHADER_HPP
