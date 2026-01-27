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
 * Renderable component - marks entity as visible.
 */
struct Renderable {
    bool visible;
    
    Renderable() : visible(true) {}
};

/**
 * Color component - RGBA color for entity.
 */
struct Color {
    float r, g, b, a;
    
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f)
        : r(r_), g(g_), b(b_), a(a_) {}
};

/**
 * TrackTrail component - stores position history for trails.
 * Uses a circular buffer to avoid reallocation.
 */
struct TrackTrail {
    std::vector<float> positions; // x,y,z triplets in circular buffer
    uint32_t max_points;           // Maximum trail length
    uint32_t current_count;        // Current number of points
    uint32_t head_index;           // Index of newest point
    
    TrackTrail(uint32_t max_trail_points = 100)
        : max_points(max_trail_points)
        , current_count(0)
        , head_index(0)
    {
        positions.resize(max_points * 3); // Pre-allocate
    }
    
    void add_point(float x, float y, float z) {
        // Add to circular buffer
        head_index = (head_index + 1) % max_points;
        uint32_t base_idx = head_index * 3;
        positions[base_idx + 0] = x;
        positions[base_idx + 1] = y;
        positions[base_idx + 2] = z;
        
        if (current_count < max_points) {
            current_count++;
        }
    }
    
    void clear() {
        current_count = 0;
        head_index = 0;
    }
};

/**
 * LabelRef component - reference to entity label (ID/handle).
 */
struct LabelRef {
    uint32_t label_id;
    
    LabelRef() : label_id(0) {}
    explicit LabelRef(uint32_t id) : label_id(id) {}
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
     * Ensure entity exists with specific ID (for ingest/WorldSync).
     * If entity doesn't exist, creates it. If it exists, does nothing.
     * Returns true if entity was created, false if it already existed.
     */
    bool ensure_entity(uint32_t entity_id);

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
     * Set entity renderable component.
     */
    void set_entity_renderable(uint32_t entity_id, bool visible);
    
    /**
     * Get entity renderable component.
     */
    const Renderable* get_entity_renderable(uint32_t entity_id) const;
    
    /**
     * Set entity color component.
     */
    void set_entity_color(uint32_t entity_id, float r, float g, float b, float a = 1.0f);
    
    /**
     * Get entity color component.
     */
    const Color* get_entity_color(uint32_t entity_id) const;
    
    /**
     * Set entity trail component (max trail points).
     */
    void set_entity_trail(uint32_t entity_id, uint32_t max_points);
    
    /**
     * Get entity trail component.
     */
    const TrackTrail* get_entity_trail(uint32_t entity_id) const;
    
    /**
     * Add position to entity trail.
     */
    void add_entity_trail_point(uint32_t entity_id, float x, float y, float z);
    
    /**
     * Set entity label reference.
     */
    void set_entity_label(uint32_t entity_id, uint32_t label_id);
    
    /**
     * Get entity label reference.
     */
    const LabelRef* get_entity_label(uint32_t entity_id) const;
    
    /**
     * Apply entity snapshot at time t (WorldSync entry point).
     */
    void apply_entity_snapshot(uint32_t entity_id, float pos_x, float pos_y, float pos_z);
    
    /**
     * Get all entities with renderable component (for rendering).
     */
    const std::vector<uint32_t>& get_renderable_entities() const;

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
    std::unordered_map<uint32_t, Renderable> renderables_;
    std::unordered_map<uint32_t, Color> colors_;
    std::unordered_map<uint32_t, TrackTrail> trails_;
    std::unordered_map<uint32_t, LabelRef> labels_;
    std::vector<uint32_t> active_entities_;
    std::vector<uint32_t> renderable_entities_cache_;
    
    Camera camera_;
    bool is_initialized_;
};

} // namespace astraeus

#endif // ASTRAEUS_WORLD_HPP
