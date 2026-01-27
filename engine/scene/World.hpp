#ifndef ASTRAEUS_WORLD_HPP
#define ASTRAEUS_WORLD_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Camera.hpp"

namespace astraeus {

/**
 * Entity transform (SoA component storage).
 */
struct Transform {
    float pos_x, pos_y, pos_z;
    float rot_x, rot_y, rot_z;
    float scale_x, scale_y, scale_z;

    Transform()
        : pos_x(0), pos_y(0), pos_z(0)
        , rot_x(0), rot_y(0), rot_z(0)
        , scale_x(1), scale_y(1), scale_z(1)
    {}
};

/**
 * World manages the scene graph with handle-based entities.
 * Uses data-oriented design with SoA layouts.
 */
class World {
public:
    World();
    ~World();

    bool initialize();
    void shutdown();

    /**
     * Create a new entity and return its handle.
     */
    uint32_t create_entity();

    /**
     * Destroy an entity.
     */
    void destroy_entity(uint32_t entity_id);

    /**
     * Get entity count.
     */
    uint32_t get_entity_count() const;

    /**
     * Set entity transform.
     */
    void set_entity_transform(uint32_t entity_id,
                             float pos_x, float pos_y, float pos_z,
                             float rot_x, float rot_y, float rot_z,
                             float scale_x, float scale_y, float scale_z);

    /**
     * Get entity transform.
     */
    const Transform* get_entity_transform(uint32_t entity_id) const;

    /**
     * Set camera state.
     */
    void set_camera(float eye_x, float eye_y, float eye_z,
                   float target_x, float target_y, float target_z,
                   float up_x, float up_y, float up_z);

    /**
     * Set camera projection.
     */
    void set_camera_projection(float fov_degrees, float near_plane, float far_plane);

    /**
     * Get camera object.
     */
    Camera& get_camera() { return camera_; }
    const Camera& get_camera() const { return camera_; }
    
    /**
     * Update camera matrices (call before rendering).
     */
    void update_camera(float aspect_ratio);

private:
    // Handle-based entity storage
    uint32_t next_entity_id_;
    std::unordered_map<uint32_t, Transform> transforms_;
    std::vector<uint32_t> active_entities_;
    
    Camera camera_;
    bool is_initialized_;
};

} // namespace astraeus

#endif // ASTRAEUS_WORLD_HPP
