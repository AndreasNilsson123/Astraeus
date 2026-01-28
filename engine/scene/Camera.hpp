#ifndef ASTRAEUS_CAMERA_HPP
#define ASTRAEUS_CAMERA_HPP

#include <cstdint>
#include <cmath>
#include <cstring>

namespace astraeus {

// Constants
constexpr float PI = 3.14159265359f;
constexpr float MAX_ELEVATION_DEGREES = 89.0f;

// Helper functions for matrix math
namespace {

inline void mat4_identity(float* m) {
    std::memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

inline void mat4_multiply(float* result, const float* a, const float* b) {
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

inline void vec3_normalize(float& x, float& y, float& z) {
    float length = std::sqrt(x * x + y * y + z * z);
    if (length > 1e-6f) {
        x /= length;
        y /= length;
        z /= length;
    }
}

inline void vec3_cross(float& out_x, float& out_y, float& out_z,
                      float a_x, float a_y, float a_z,
                      float b_x, float b_y, float b_z) {
    out_x = a_y * b_z - a_z * b_y;
    out_y = a_z * b_x - a_x * b_z;
    out_z = a_x * b_y - a_y * b_x;
}

} // anonymous namespace

/**
 * Camera with orbit/pan/zoom capabilities.
 * Manages view and projection matrices.
 */
class Camera {
public:
    inline Camera();
    ~Camera() = default;

    /**
     * Set camera position, target, and up vector.
     */
    inline void set_view(float eye_x, float eye_y, float eye_z,
                  float target_x, float target_y, float target_z,
                  float up_x, float up_y, float up_z);

    /**
     * Set perspective projection parameters.
     */
    inline void set_projection(float fov_degrees, float aspect_ratio, 
                       float near_plane, float far_plane);

    /**
     * Orbit around target (in degrees).
     */
    inline void orbit(float delta_azimuth, float delta_elevation);

    /**
     * Pan camera and target together.
     */
    inline void pan(float delta_x, float delta_y, float delta_z);

    /**
     * Zoom by moving closer/farther from target.
     */
    inline void zoom(float delta_distance);

    /**
     * Update matrices based on current state.
     */
    inline void update_matrices(float aspect_ratio);

    // Getters
    const float* get_view_matrix() const { return view_matrix_; }
    const float* get_projection_matrix() const { return projection_matrix_; }
    const float* get_view_projection_matrix() const { return view_projection_matrix_; }

    void get_position(float& out_x, float& out_y, float& out_z) const {
        out_x = eye_x_;
        out_y = eye_y_;
        out_z = eye_z_;
    }

    void get_target(float& out_x, float& out_y, float& out_z) const {
        out_x = target_x_;
        out_y = target_y_;
        out_z = target_z_;
    }

    float get_fov() const { return fov_degrees_; }
    float get_near_plane() const { return near_plane_; }
    float get_far_plane() const { return far_plane_; }

private:
    inline void compute_view_matrix();
    inline void compute_projection_matrix(float aspect_ratio);
    inline void compute_view_projection_matrix();

    // Camera state
    float eye_x_, eye_y_, eye_z_;
    float target_x_, target_y_, target_z_;
    float up_x_, up_y_, up_z_;

    // Projection parameters
    float fov_degrees_;
    float near_plane_;
    float far_plane_;

    // Matrices (column-major, OpenGL style)
    float view_matrix_[16];
    float projection_matrix_[16];
    float view_projection_matrix_[16];

    bool view_dirty_;
    bool projection_dirty_;
};

// Implementation

inline Camera::Camera()
    : eye_x_(10.0f), eye_y_(10.0f), eye_z_(10.0f)
    , target_x_(0.0f), target_y_(0.0f), target_z_(0.0f)
    , up_x_(0.0f), up_y_(1.0f), up_z_(0.0f)
    , fov_degrees_(60.0f)
    , near_plane_(0.1f)
    , far_plane_(1000.0f)
    , view_dirty_(true)
    , projection_dirty_(true)
{
    mat4_identity(view_matrix_);
    mat4_identity(projection_matrix_);
    mat4_identity(view_projection_matrix_);
}

inline void Camera::set_view(float eye_x, float eye_y, float eye_z,
                     float target_x, float target_y, float target_z,
                     float up_x, float up_y, float up_z) {
    eye_x_ = eye_x;
    eye_y_ = eye_y;
    eye_z_ = eye_z;
    target_x_ = target_x;
    target_y_ = target_y;
    target_z_ = target_z;
    up_x_ = up_x;
    up_y_ = up_y;
    up_z_ = up_z;
    view_dirty_ = true;
}

inline void Camera::set_projection(float fov_degrees, float aspect_ratio,
                           float near_plane, float far_plane) {
    fov_degrees_ = fov_degrees;
    near_plane_ = near_plane;
    far_plane_ = far_plane;
    projection_dirty_ = true;
    compute_projection_matrix(aspect_ratio);
}

inline void Camera::orbit(float delta_azimuth, float delta_elevation) {
    // Convert angles to radians
    float azimuth_rad = delta_azimuth * PI / 180.0f;
    float elevation_rad = delta_elevation * PI / 180.0f;

    // Compute vector from target to eye
    float dx = eye_x_ - target_x_;
    float dy = eye_y_ - target_y_;
    float dz = eye_z_ - target_z_;
    
    float radius = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    // Convert to spherical coordinates
    float current_azimuth = std::atan2(dx, dz);
    float horizontal_dist = std::sqrt(dx * dx + dz * dz);
    float current_elevation = std::atan2(dy, horizontal_dist);
    
    // Apply deltas
    current_azimuth += azimuth_rad;
    current_elevation += elevation_rad;
    
    // Clamp elevation to avoid gimbal lock
    const float max_elevation = MAX_ELEVATION_DEGREES * PI / 180.0f;
    if (current_elevation > max_elevation) current_elevation = max_elevation;
    if (current_elevation < -max_elevation) current_elevation = -max_elevation;
    
    // Convert back to cartesian
    horizontal_dist = radius * std::cos(current_elevation);
    eye_x_ = target_x_ + horizontal_dist * std::sin(current_azimuth);
    eye_y_ = target_y_ + radius * std::sin(current_elevation);
    eye_z_ = target_z_ + horizontal_dist * std::cos(current_azimuth);
    
    view_dirty_ = true;
}

inline void Camera::pan(float delta_x, float delta_y, float delta_z) {
    // Compute camera right and up vectors
    float forward_x = target_x_ - eye_x_;
    float forward_y = target_y_ - eye_y_;
    float forward_z = target_z_ - eye_z_;
    vec3_normalize(forward_x, forward_y, forward_z);
    
    // Right vector = forward x world_up
    float right_x, right_y, right_z;
    vec3_cross(right_x, right_y, right_z,
               forward_x, forward_y, forward_z,
               0.0f, 1.0f, 0.0f);
    vec3_normalize(right_x, right_y, right_z);
    
    // Up vector = right x forward
    float cam_up_x, cam_up_y, cam_up_z;
    vec3_cross(cam_up_x, cam_up_y, cam_up_z,
               right_x, right_y, right_z,
               forward_x, forward_y, forward_z);
    
    // Apply panning
    eye_x_ += right_x * delta_x + cam_up_x * delta_y + forward_x * delta_z;
    eye_y_ += right_y * delta_x + cam_up_y * delta_y + forward_y * delta_z;
    eye_z_ += right_z * delta_x + cam_up_z * delta_y + forward_z * delta_z;
    
    target_x_ += right_x * delta_x + cam_up_x * delta_y + forward_x * delta_z;
    target_y_ += right_y * delta_x + cam_up_y * delta_y + forward_y * delta_z;
    target_z_ += right_z * delta_x + cam_up_z * delta_y + forward_z * delta_z;
    
    view_dirty_ = true;
}

inline void Camera::zoom(float delta_distance) {
    // Move along the view direction
    float dx = eye_x_ - target_x_;
    float dy = eye_y_ - target_y_;
    float dz = eye_z_ - target_z_;
    
    float current_distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    float new_distance = current_distance - delta_distance;
    
    // Clamp to reasonable limits
    if (new_distance < 0.5f) new_distance = 0.5f;
    if (new_distance > 500.0f) new_distance = 500.0f;
    
    if (current_distance > 1e-6f) {
        float scale = new_distance / current_distance;
        eye_x_ = target_x_ + dx * scale;
        eye_y_ = target_y_ + dy * scale;
        eye_z_ = target_z_ + dz * scale;
    }
    
    view_dirty_ = true;
}

inline void Camera::update_matrices(float aspect_ratio) {
    if (view_dirty_) {
        compute_view_matrix();
        view_dirty_ = false;
    }
    
    if (projection_dirty_) {
        compute_projection_matrix(aspect_ratio);
        projection_dirty_ = false;
    }
    
    compute_view_projection_matrix();
}

inline void Camera::compute_view_matrix() {
    // Build look-at matrix
    float forward_x = target_x_ - eye_x_;
    float forward_y = target_y_ - eye_y_;
    float forward_z = target_z_ - eye_z_;
    vec3_normalize(forward_x, forward_y, forward_z);
    
    // Right = forward x up
    float right_x, right_y, right_z;
    vec3_cross(right_x, right_y, right_z,
               forward_x, forward_y, forward_z,
               up_x_, up_y_, up_z_);
    vec3_normalize(right_x, right_y, right_z);
    
    // Recompute up = right x forward
    float cam_up_x, cam_up_y, cam_up_z;
    vec3_cross(cam_up_x, cam_up_y, cam_up_z,
               right_x, right_y, right_z,
               forward_x, forward_y, forward_z);
    
    // Build view matrix (column-major)
    view_matrix_[0] = right_x;
    view_matrix_[1] = cam_up_x;
    view_matrix_[2] = -forward_x;
    view_matrix_[3] = 0.0f;
    
    view_matrix_[4] = right_y;
    view_matrix_[5] = cam_up_y;
    view_matrix_[6] = -forward_y;
    view_matrix_[7] = 0.0f;
    
    view_matrix_[8] = right_z;
    view_matrix_[9] = cam_up_z;
    view_matrix_[10] = -forward_z;
    view_matrix_[11] = 0.0f;
    
    view_matrix_[12] = -(right_x * eye_x_ + right_y * eye_y_ + right_z * eye_z_);
    view_matrix_[13] = -(cam_up_x * eye_x_ + cam_up_y * eye_y_ + cam_up_z * eye_z_);
    view_matrix_[14] = -(-forward_x * eye_x_ + -forward_y * eye_y_ + -forward_z * eye_z_);
    view_matrix_[15] = 1.0f;
}

inline void Camera::compute_projection_matrix(float aspect_ratio) {
    // Perspective projection matrix
    float fov_rad = fov_degrees_ * PI / 180.0f;
    float tan_half_fov = std::tan(fov_rad / 2.0f);
    
    std::memset(projection_matrix_, 0, 16 * sizeof(float));
    
    projection_matrix_[0] = 1.0f / (aspect_ratio * tan_half_fov);
    projection_matrix_[5] = 1.0f / tan_half_fov;
    projection_matrix_[10] = -(far_plane_ + near_plane_) / (far_plane_ - near_plane_);
    projection_matrix_[11] = -1.0f;
    projection_matrix_[14] = -(2.0f * far_plane_ * near_plane_) / (far_plane_ - near_plane_);
}

inline void Camera::compute_view_projection_matrix() {
    mat4_multiply(view_projection_matrix_, projection_matrix_, view_matrix_);
}

} // namespace astraeus

#endif // ASTRAEUS_CAMERA_HPP
