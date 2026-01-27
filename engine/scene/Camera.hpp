#ifndef ASTRAEUS_CAMERA_HPP
#define ASTRAEUS_CAMERA_HPP

#include <cstdint>

namespace astraeus {

/**
 * Camera with orbit/pan/zoom capabilities.
 * Manages view and projection matrices.
 */
class Camera {
public:
    Camera();
    ~Camera() = default;

    /**
     * Set camera position, target, and up vector.
     */
    void set_view(float eye_x, float eye_y, float eye_z,
                  float target_x, float target_y, float target_z,
                  float up_x, float up_y, float up_z);

    /**
     * Set perspective projection parameters.
     */
    void set_projection(float fov_degrees, float aspect_ratio, 
                       float near_plane, float far_plane);

    /**
     * Orbit around target (in degrees).
     */
    void orbit(float delta_azimuth, float delta_elevation);

    /**
     * Pan camera and target together.
     */
    void pan(float delta_x, float delta_y, float delta_z);

    /**
     * Zoom by moving closer/farther from target.
     */
    void zoom(float delta_distance);

    /**
     * Update matrices based on current state.
     */
    void update_matrices(float aspect_ratio);

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
    void compute_view_matrix();
    void compute_projection_matrix(float aspect_ratio);
    void compute_view_projection_matrix();

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

} // namespace astraeus

#endif // ASTRAEUS_CAMERA_HPP
