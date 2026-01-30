#ifndef ASTRAEUS_CAMERA_COMPONENT_HPP
#define ASTRAEUS_CAMERA_COMPONENT_HPP

#include <cstdint>
#include <cmath>
#include <cstring>

namespace astraeus {

/**
 * Camera projection type.
 */
enum class CameraProjectionType : uint8_t {
    Perspective,
    Orthographic
};

/**
 * Frustum planes for frustum culling.
 * Planes are stored in Ax + By + Cz + D = 0 form (normalized).
 */
struct Frustum {
    // 6 planes: left, right, bottom, top, near, far
    // Each plane: [A, B, C, D] where Ax + By + Cz + D = 0
    float planes[6][4];
    
    /**
     * Test if a point is inside the frustum.
     */
    inline bool contains_point(float x, float y, float z) const {
        for (int i = 0; i < 6; ++i) {
            float distance = planes[i][0] * x + planes[i][1] * y + planes[i][2] * z + planes[i][3];
            if (distance < 0.0f) {
                return false;
            }
        }
        return true;
    }
    
    /**
     * Test if a sphere intersects the frustum.
     */
    inline bool intersects_sphere(float x, float y, float z, float radius) const {
        for (int i = 0; i < 6; ++i) {
            float distance = planes[i][0] * x + planes[i][1] * y + planes[i][2] * z + planes[i][3];
            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }
    
    /**
     * Test if an AABB intersects the frustum.
     */
    inline bool intersects_aabb(float min_x, float min_y, float min_z,
                                float max_x, float max_y, float max_z) const {
        for (int i = 0; i < 6; ++i) {
            // Find the positive vertex (the one furthest along plane normal)
            float px = (planes[i][0] >= 0.0f) ? max_x : min_x;
            float py = (planes[i][1] >= 0.0f) ? max_y : min_y;
            float pz = (planes[i][2] >= 0.0f) ? max_z : min_z;
            
            float distance = planes[i][0] * px + planes[i][1] * py + planes[i][2] * pz + planes[i][3];
            if (distance < 0.0f) {
                return false;
            }
        }
        return true;
    }
};

/**
 * Camera component - can be attached to entities in the scene.
 * Supports both perspective and orthographic projection.
 */
struct CameraComponent {
    // Camera type
    CameraProjectionType projection_type;
    
    // Perspective parameters
    float fov_degrees;           // Vertical field of view (perspective only)
    
    // Orthographic parameters
    float ortho_left;
    float ortho_right;
    float ortho_bottom;
    float ortho_top;
    
    // Common parameters
    float near_plane;
    float far_plane;
    
    // Rendering parameters (placeholders for future use)
    float exposure;              // Exposure value for HDR rendering
    
    // Computed matrices (column-major, OpenGL style)
    float view_matrix[16];
    float projection_matrix[16];
    float view_projection_matrix[16];
    
    // Frustum for culling
    Frustum frustum;
    
    // Flags
    bool view_dirty;
    bool projection_dirty;
    bool is_active;              // Whether this camera is actively used for rendering
    
    CameraComponent()
        : projection_type(CameraProjectionType::Perspective)
        , fov_degrees(60.0f)
        , ortho_left(-10.0f)
        , ortho_right(10.0f)
        , ortho_bottom(-10.0f)
        , ortho_top(10.0f)
        , near_plane(0.1f)
        , far_plane(1000.0f)
        , exposure(1.0f)
        , view_dirty(true)
        , projection_dirty(true)
        , is_active(false)
    {
        // Initialize to identity matrices
        std::memset(view_matrix, 0, 16 * sizeof(float));
        std::memset(projection_matrix, 0, 16 * sizeof(float));
        std::memset(view_projection_matrix, 0, 16 * sizeof(float));
        view_matrix[0] = view_matrix[5] = view_matrix[10] = view_matrix[15] = 1.0f;
        projection_matrix[0] = projection_matrix[5] = projection_matrix[10] = projection_matrix[15] = 1.0f;
        view_projection_matrix[0] = view_projection_matrix[5] = view_projection_matrix[10] = view_projection_matrix[15] = 1.0f;
        
        // Initialize frustum planes
        std::memset(&frustum, 0, sizeof(Frustum));
    }
};

/**
 * Camera uniform data pack for renderer.
 * This is a minimal, renderer-friendly structure that can be uploaded to GPU.
 */
struct CameraUniforms {
    float view_matrix[16];           // View matrix
    float projection_matrix[16];     // Projection matrix
    float view_projection_matrix[16]; // Combined VP matrix
    float camera_position[3];        // Camera world position
    float _padding0;                 // Padding for alignment
    float camera_direction[3];       // Camera forward direction
    float _padding1;                 // Padding for alignment
    float near_plane;
    float far_plane;
    float fov_degrees;               // For perspective cameras
    float exposure;
    
    CameraUniforms() {
        std::memset(this, 0, sizeof(CameraUniforms));
        // Default identity matrices
        view_matrix[0] = view_matrix[5] = view_matrix[10] = view_matrix[15] = 1.0f;
        projection_matrix[0] = projection_matrix[5] = projection_matrix[10] = projection_matrix[15] = 1.0f;
        view_projection_matrix[0] = view_projection_matrix[5] = view_projection_matrix[10] = view_projection_matrix[15] = 1.0f;
        near_plane = 0.1f;
        far_plane = 1000.0f;
        fov_degrees = 60.0f;
        exposure = 1.0f;
    }
};

} // namespace astraeus

#endif // ASTRAEUS_CAMERA_COMPONENT_HPP
