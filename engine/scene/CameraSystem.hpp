#ifndef ASTRAEUS_CAMERA_SYSTEM_HPP
#define ASTRAEUS_CAMERA_SYSTEM_HPP

#include "CameraComponent.hpp"
#include <cmath>
#include <cstring>
#include <iostream>

namespace astraeus {

/**
 * CameraSystem manages camera components and provides utilities
 * for camera matrix computation and frustum extraction.
 */
class CameraSystem {
public:
    /**
     * Update camera view matrix from position and orientation.
     * Uses look-at matrix construction.
     */
    static inline void update_view_matrix(CameraComponent& camera,
                                         float eye_x, float eye_y, float eye_z,
                                         float target_x, float target_y, float target_z,
                                         float up_x, float up_y, float up_z);
    
    /**
     * Update camera projection matrix based on aspect ratio.
     */
    static inline void update_projection_matrix(CameraComponent& camera, float aspect_ratio);
    
    /**
     * Compute view-projection matrix from view and projection matrices.
     */
    static inline void update_view_projection_matrix(CameraComponent& camera);
    
    /**
     * Extract frustum planes from view-projection matrix.
     */
    static inline void extract_frustum(CameraComponent& camera);
    
    /**
     * Update all camera matrices and frustum.
     */
    static inline void update_camera(CameraComponent& camera,
                                    float eye_x, float eye_y, float eye_z,
                                    float target_x, float target_y, float target_z,
                                    float up_x, float up_y, float up_z,
                                    float aspect_ratio);
    
    /**
     * Build camera uniforms pack for rendering.
     */
    static inline void build_camera_uniforms(const CameraComponent& camera,
                                            float pos_x, float pos_y, float pos_z,
                                            CameraUniforms& uniforms);
    
    /**
     * Set perspective projection parameters.
     */
    static inline void set_perspective(CameraComponent& camera,
                                      float fov_degrees,
                                      float near_plane,
                                      float far_plane);
    
    /**
     * Set orthographic projection parameters.
     */
    static inline void set_orthographic(CameraComponent& camera,
                                       float left, float right,
                                       float bottom, float top,
                                       float near_plane, float far_plane);

private:
    static inline void mat4_multiply(const float* a, const float* b, float* result);
    static inline void vec3_normalize(float& x, float& y, float& z);
    static inline void vec3_cross(float& out_x, float& out_y, float& out_z,
                                 float a_x, float a_y, float a_z,
                                 float b_x, float b_y, float b_z);
};

// ============================================================================
// Implementation
// ============================================================================

inline void CameraSystem::vec3_normalize(float& x, float& y, float& z) {
    float length = std::sqrt(x * x + y * y + z * z);
    if (length > 1e-6f) {
        x /= length;
        y /= length;
        z /= length;
    }
}

inline void CameraSystem::vec3_cross(float& out_x, float& out_y, float& out_z,
                                     float a_x, float a_y, float a_z,
                                     float b_x, float b_y, float b_z) {
    out_x = a_y * b_z - a_z * b_y;
    out_y = a_z * b_x - a_x * b_z;
    out_z = a_x * b_y - a_y * b_x;
}

inline void CameraSystem::mat4_multiply(const float* a, const float* b, float* result) {
    float temp[16];
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            temp[col * 4 + row] = 
                a[0 * 4 + row] * b[col * 4 + 0] +
                a[1 * 4 + row] * b[col * 4 + 1] +
                a[2 * 4 + row] * b[col * 4 + 2] +
                a[3 * 4 + row] * b[col * 4 + 3];
        }
    }
    std::memcpy(result, temp, 16 * sizeof(float));
}

inline void CameraSystem::update_view_matrix(CameraComponent& camera,
                                            float eye_x, float eye_y, float eye_z,
                                            float target_x, float target_y, float target_z,
                                            float up_x, float up_y, float up_z) {
    // Build look-at matrix
    float forward_x = target_x - eye_x;
    float forward_y = target_y - eye_y;
    float forward_z = target_z - eye_z;
    vec3_normalize(forward_x, forward_y, forward_z);
    
    // Right = forward x up
    float right_x, right_y, right_z;
    vec3_cross(right_x, right_y, right_z,
               forward_x, forward_y, forward_z,
               up_x, up_y, up_z);
    vec3_normalize(right_x, right_y, right_z);
    
    // Recompute up = right x forward
    float cam_up_x, cam_up_y, cam_up_z;
    vec3_cross(cam_up_x, cam_up_y, cam_up_z,
               right_x, right_y, right_z,
               forward_x, forward_y, forward_z);
    
    // Build view matrix (column-major)
    camera.view_matrix[0] = right_x;
    camera.view_matrix[1] = cam_up_x;
    camera.view_matrix[2] = -forward_x;
    camera.view_matrix[3] = 0.0f;
    
    camera.view_matrix[4] = right_y;
    camera.view_matrix[5] = cam_up_y;
    camera.view_matrix[6] = -forward_y;
    camera.view_matrix[7] = 0.0f;
    
    camera.view_matrix[8] = right_z;
    camera.view_matrix[9] = cam_up_z;
    camera.view_matrix[10] = -forward_z;
    camera.view_matrix[11] = 0.0f;
    
    camera.view_matrix[12] = -(right_x * eye_x + right_y * eye_y + right_z * eye_z);
    camera.view_matrix[13] = -(cam_up_x * eye_x + cam_up_y * eye_y + cam_up_z * eye_z);
    camera.view_matrix[14] = -(-forward_x * eye_x + -forward_y * eye_y + -forward_z * eye_z);
    camera.view_matrix[15] = 1.0f;
    
    camera.view_dirty = false;
}

inline void CameraSystem::update_projection_matrix(CameraComponent& camera, float aspect_ratio) {
    constexpr float PI = 3.14159265359f;
    constexpr float EPSILON = 1e-6f;
    
    std::memset(camera.projection_matrix, 0, 16 * sizeof(float));
    
    if (camera.projection_type == CameraProjectionType::Perspective) {
        // Validate parameters
        if (aspect_ratio <= EPSILON) {
            std::cerr << "[CameraSystem] Warning: aspect_ratio must be > 0, using 1.0" << std::endl;
            aspect_ratio = 1.0f;
        }
        if (camera.fov_degrees <= EPSILON || camera.fov_degrees >= 180.0f) {
            std::cerr << "[CameraSystem] Warning: fov_degrees must be in (0, 180), clamping" << std::endl;
            camera.fov_degrees = std::max(1.0f, std::min(179.0f, camera.fov_degrees));
        }
        if (camera.far_plane <= camera.near_plane + EPSILON) {
            std::cerr << "[CameraSystem] Warning: far_plane must be > near_plane" << std::endl;
            camera.far_plane = camera.near_plane + 1.0f;
        }
        
        // Perspective projection
        float fov_rad = camera.fov_degrees * PI / 180.0f;
        float tan_half_fov = std::tan(fov_rad / 2.0f);
        
        camera.projection_matrix[0] = 1.0f / (aspect_ratio * tan_half_fov);
        camera.projection_matrix[5] = 1.0f / tan_half_fov;
        camera.projection_matrix[10] = -(camera.far_plane + camera.near_plane) / (camera.far_plane - camera.near_plane);
        camera.projection_matrix[11] = -1.0f;
        camera.projection_matrix[14] = -(2.0f * camera.far_plane * camera.near_plane) / (camera.far_plane - camera.near_plane);
    } else {
        // Validate parameters
        float width = camera.ortho_right - camera.ortho_left;
        float height = camera.ortho_top - camera.ortho_bottom;
        float depth = camera.far_plane - camera.near_plane;
        
        if (std::abs(width) <= EPSILON) {
            std::cerr << "[CameraSystem] Warning: ortho_right must != ortho_left" << std::endl;
            camera.ortho_right = camera.ortho_left + 1.0f;
            width = 1.0f;
        }
        if (std::abs(height) <= EPSILON) {
            std::cerr << "[CameraSystem] Warning: ortho_top must != ortho_bottom" << std::endl;
            camera.ortho_top = camera.ortho_bottom + 1.0f;
            height = 1.0f;
        }
        if (std::abs(depth) <= EPSILON) {
            std::cerr << "[CameraSystem] Warning: far_plane must != near_plane" << std::endl;
            camera.far_plane = camera.near_plane + 1.0f;
            depth = 1.0f;
        }
        
        // Orthographic projection
        camera.projection_matrix[0] = 2.0f / width;
        camera.projection_matrix[5] = 2.0f / height;
        camera.projection_matrix[10] = -2.0f / depth;
        camera.projection_matrix[12] = -(camera.ortho_right + camera.ortho_left) / width;
        camera.projection_matrix[13] = -(camera.ortho_top + camera.ortho_bottom) / height;
        camera.projection_matrix[14] = -(camera.far_plane + camera.near_plane) / depth;
        camera.projection_matrix[15] = 1.0f;
    }
    
    camera.projection_dirty = false;
}

inline void CameraSystem::update_view_projection_matrix(CameraComponent& camera) {
    mat4_multiply(camera.projection_matrix, camera.view_matrix, camera.view_projection_matrix);
}

inline void CameraSystem::extract_frustum(CameraComponent& camera) {
    // Extract frustum planes from view-projection matrix
    // The matrix contains the planes in homogeneous coordinates
    const float* m = camera.view_projection_matrix;
    
    // Left plane: m[3] + m[0]
    camera.frustum.planes[0][0] = m[3] + m[0];
    camera.frustum.planes[0][1] = m[7] + m[4];
    camera.frustum.planes[0][2] = m[11] + m[8];
    camera.frustum.planes[0][3] = m[15] + m[12];
    
    // Right plane: m[3] - m[0]
    camera.frustum.planes[1][0] = m[3] - m[0];
    camera.frustum.planes[1][1] = m[7] - m[4];
    camera.frustum.planes[1][2] = m[11] - m[8];
    camera.frustum.planes[1][3] = m[15] - m[12];
    
    // Bottom plane: m[3] + m[1]
    camera.frustum.planes[2][0] = m[3] + m[1];
    camera.frustum.planes[2][1] = m[7] + m[5];
    camera.frustum.planes[2][2] = m[11] + m[9];
    camera.frustum.planes[2][3] = m[15] + m[13];
    
    // Top plane: m[3] - m[1]
    camera.frustum.planes[3][0] = m[3] - m[1];
    camera.frustum.planes[3][1] = m[7] - m[5];
    camera.frustum.planes[3][2] = m[11] - m[9];
    camera.frustum.planes[3][3] = m[15] - m[13];
    
    // Near plane: m[3] + m[2]
    camera.frustum.planes[4][0] = m[3] + m[2];
    camera.frustum.planes[4][1] = m[7] + m[6];
    camera.frustum.planes[4][2] = m[11] + m[10];
    camera.frustum.planes[4][3] = m[15] + m[14];
    
    // Far plane: m[3] - m[2]
    camera.frustum.planes[5][0] = m[3] - m[2];
    camera.frustum.planes[5][1] = m[7] - m[6];
    camera.frustum.planes[5][2] = m[11] - m[10];
    camera.frustum.planes[5][3] = m[15] - m[14];
    
    // Normalize all planes
    for (int i = 0; i < 6; ++i) {
        float length = std::sqrt(
            camera.frustum.planes[i][0] * camera.frustum.planes[i][0] +
            camera.frustum.planes[i][1] * camera.frustum.planes[i][1] +
            camera.frustum.planes[i][2] * camera.frustum.planes[i][2]
        );
        if (length > 1e-6f) {
            camera.frustum.planes[i][0] /= length;
            camera.frustum.planes[i][1] /= length;
            camera.frustum.planes[i][2] /= length;
            camera.frustum.planes[i][3] /= length;
        }
    }
}

inline void CameraSystem::update_camera(CameraComponent& camera,
                                       float eye_x, float eye_y, float eye_z,
                                       float target_x, float target_y, float target_z,
                                       float up_x, float up_y, float up_z,
                                       float aspect_ratio) {
    update_view_matrix(camera, eye_x, eye_y, eye_z, target_x, target_y, target_z, up_x, up_y, up_z);
    update_projection_matrix(camera, aspect_ratio);
    update_view_projection_matrix(camera);
    extract_frustum(camera);
}

inline void CameraSystem::build_camera_uniforms(const CameraComponent& camera,
                                               float pos_x, float pos_y, float pos_z,
                                               CameraUniforms& uniforms) {
    // Copy matrices
    std::memcpy(uniforms.view_matrix, camera.view_matrix, 16 * sizeof(float));
    std::memcpy(uniforms.projection_matrix, camera.projection_matrix, 16 * sizeof(float));
    std::memcpy(uniforms.view_projection_matrix, camera.view_projection_matrix, 16 * sizeof(float));
    
    // Set camera position
    uniforms.camera_position[0] = pos_x;
    uniforms.camera_position[1] = pos_y;
    uniforms.camera_position[2] = pos_z;
    
    // Extract camera forward direction from view matrix
    // The view matrix transforms from world to view space.
    // The third column (negated) gives the camera's forward direction in world space.
    uniforms.camera_direction[0] = -camera.view_matrix[2];
    uniforms.camera_direction[1] = -camera.view_matrix[6];
    uniforms.camera_direction[2] = -camera.view_matrix[10];
    
    // Set other parameters
    uniforms.near_plane = camera.near_plane;
    uniforms.far_plane = camera.far_plane;
    uniforms.fov_degrees = camera.fov_degrees;
    uniforms.exposure = camera.exposure;
}

inline void CameraSystem::set_perspective(CameraComponent& camera,
                                         float fov_degrees,
                                         float near_plane,
                                         float far_plane) {
    camera.projection_type = CameraProjectionType::Perspective;
    camera.fov_degrees = fov_degrees;
    camera.near_plane = near_plane;
    camera.far_plane = far_plane;
    camera.projection_dirty = true;
}

inline void CameraSystem::set_orthographic(CameraComponent& camera,
                                          float left, float right,
                                          float bottom, float top,
                                          float near_plane, float far_plane) {
    camera.projection_type = CameraProjectionType::Orthographic;
    camera.ortho_left = left;
    camera.ortho_right = right;
    camera.ortho_bottom = bottom;
    camera.ortho_top = top;
    camera.near_plane = near_plane;
    camera.far_plane = far_plane;
    camera.projection_dirty = true;
}

} // namespace astraeus

#endif // ASTRAEUS_CAMERA_SYSTEM_HPP
