#ifndef ASTRAEUS_WORLD_HPP
#define ASTRAEUS_WORLD_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
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

// ============================================================================
// Implementation (header-only)
// ============================================================================

inline World::World()
    : next_entity_id_(1) // Start at 1, 0 is reserved for "null"
    , is_initialized_(false)
{
}

inline World::~World() {
    shutdown();
}

inline bool World::initialize() {
    if (is_initialized_) {
        return true;
    }

    std::cout << "[World] Initializing world" << std::endl;
    
    is_initialized_ = true;
    return true;
}

inline void World::shutdown() {
    if (!is_initialized_) {
        return;
    }

    std::cout << "[World] Shutting down" << std::endl;
    
    transforms_.clear();
    renderables_.clear();
    colors_.clear();
    trails_.clear();
    labels_.clear();
    active_entities_.clear();
    renderable_entities_cache_.clear();
    next_entity_id_ = 1;
    
    is_initialized_ = false;
}

inline uint32_t World::create_entity() {
    uint32_t entity_id = next_entity_id_++;
    
    // Initialize with default transform
    transforms_[entity_id] = Transform();
    active_entities_.push_back(entity_id);
    
    return entity_id;
}

inline bool World::ensure_entity(uint32_t entity_id) {
    if (entity_id == 0) {
        return false;
    }
    
    if (!is_initialized_) {
        std::cerr << "[World] Warning: ensure_entity called before World is initialized" << std::endl;
        return false;
    }
    
    // Check if entity already exists
    auto it = transforms_.find(entity_id);
    if (it != transforms_.end()) {
        return false; // Already exists
    }
    
    // Create entity with specific ID
    transforms_[entity_id] = Transform();
    active_entities_.push_back(entity_id);
    
    // Update next_entity_id if needed
    // Note: This is safe because WorldSync (the only caller) runs on main thread
    // and create_entity() also runs on main thread. Not thread-safe for multi-writer scenarios.
    if (entity_id >= next_entity_id_) {
        next_entity_id_ = entity_id + 1;
    }
    
    return true; // Created
}

inline void World::destroy_entity(uint32_t entity_id) {
    if (entity_id == 0) {
        return;
    }

    // Remove all components
    transforms_.erase(entity_id);
    renderables_.erase(entity_id);
    colors_.erase(entity_id);
    trails_.erase(entity_id);
    labels_.erase(entity_id);
    
    // Remove from active list
    auto it = std::find(active_entities_.begin(), active_entities_.end(), entity_id);
    if (it != active_entities_.end()) {
        active_entities_.erase(it);
    }
    
    // Remove from renderable cache
    auto rit = std::find(renderable_entities_cache_.begin(), renderable_entities_cache_.end(), entity_id);
    if (rit != renderable_entities_cache_.end()) {
        renderable_entities_cache_.erase(rit);
    }
}

inline uint32_t World::get_entity_count() const {
    return static_cast<uint32_t>(active_entities_.size());
}

inline void World::set_entity_transform(uint32_t entity_id,
                                  float pos_x, float pos_y, float pos_z,
                                  float rot_x, float rot_y, float rot_z,
                                  float scale_x, float scale_y, float scale_z) {
    if (entity_id == 0) {
        return;
    }

    auto it = transforms_.find(entity_id);
    if (it != transforms_.end()) {
        Transform& t = it->second;
        t.pos_x = pos_x; t.pos_y = pos_y; t.pos_z = pos_z;
        t.rot_x = rot_x; t.rot_y = rot_y; t.rot_z = rot_z;
        t.scale_x = scale_x; t.scale_y = scale_y; t.scale_z = scale_z;
    }
}

inline const Transform* World::get_entity_transform(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }

    auto it = transforms_.find(entity_id);
    if (it != transforms_.end()) {
        return &it->second;
    }
    return nullptr;
}

inline void World::set_camera(float eye_x, float eye_y, float eye_z,
                       float target_x, float target_y, float target_z,
                       float up_x, float up_y, float up_z) {
    camera_.set_view(eye_x, eye_y, eye_z,
                     target_x, target_y, target_z,
                     up_x, up_y, up_z);
}

inline void World::set_camera_projection(float fov_degrees, float near_plane, float far_plane) {
    // Aspect ratio will be updated in update_camera()
    camera_.set_projection(fov_degrees, 1.0f, near_plane, far_plane);
}

inline void World::update_camera(float aspect_ratio) {
    camera_.update_matrices(aspect_ratio);
}

inline void World::set_entity_renderable(uint32_t entity_id, bool visible) {
    if (entity_id == 0) {
        return;
    }
    
    auto it = renderables_.find(entity_id);
    if (it != renderables_.end()) {
        // Entity already has renderable component
        bool was_visible = it->second.visible;
        it->second.visible = visible;
        
        // Update cache based on visibility change
        if (was_visible && !visible) {
            // Remove from cache
            auto cache_it = std::find(renderable_entities_cache_.begin(), 
                                     renderable_entities_cache_.end(), 
                                     entity_id);
            if (cache_it != renderable_entities_cache_.end()) {
                renderable_entities_cache_.erase(cache_it);
            }
        } else if (!was_visible && visible) {
            // Add to cache
            renderable_entities_cache_.push_back(entity_id);
        }
    } else {
        // New renderable component
        Renderable r;
        r.visible = visible;
        renderables_[entity_id] = r;
        if (visible) {
            renderable_entities_cache_.push_back(entity_id);
        }
    }
}

inline const Renderable* World::get_entity_renderable(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = renderables_.find(entity_id);
    if (it != renderables_.end()) {
        return &it->second;
    }
    return nullptr;
}

inline void World::set_entity_color(uint32_t entity_id, float r, float g, float b, float a) {
    if (entity_id == 0) {
        return;
    }
    
    colors_[entity_id] = Color(r, g, b, a);
}

inline const Color* World::get_entity_color(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = colors_.find(entity_id);
    if (it != colors_.end()) {
        return &it->second;
    }
    return nullptr;
}

inline void World::set_entity_trail(uint32_t entity_id, uint32_t max_points) {
    if (entity_id == 0) {
        return;
    }
    
    trails_[entity_id] = TrackTrail(max_points);
}

inline const TrackTrail* World::get_entity_trail(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = trails_.find(entity_id);
    if (it != trails_.end()) {
        return &it->second;
    }
    return nullptr;
}

inline void World::add_entity_trail_point(uint32_t entity_id, float x, float y, float z) {
    if (entity_id == 0) {
        return;
    }
    
    auto it = trails_.find(entity_id);
    if (it != trails_.end()) {
        it->second.add_point(x, y, z);
    }
}

inline void World::set_entity_label(uint32_t entity_id, uint32_t label_id) {
    if (entity_id == 0) {
        return;
    }
    
    labels_[entity_id] = LabelRef(label_id);
}

inline const LabelRef* World::get_entity_label(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = labels_.find(entity_id);
    if (it != labels_.end()) {
        return &it->second;
    }
    return nullptr;
}

inline void World::apply_entity_snapshot(uint32_t entity_id, float pos_x, float pos_y, float pos_z) {
    if (entity_id == 0) {
        return;
    }
    
    // Update transform
    auto trans_it = transforms_.find(entity_id);
    if (trans_it != transforms_.end()) {
        trans_it->second.pos_x = pos_x;
        trans_it->second.pos_y = pos_y;
        trans_it->second.pos_z = pos_z;
        
        // Update trail if it exists
        auto trail_it = trails_.find(entity_id);
        if (trail_it != trails_.end()) {
            trail_it->second.add_point(pos_x, pos_y, pos_z);
        }
    }
}

inline const std::vector<uint32_t>& World::get_renderable_entities() const {
    return renderable_entities_cache_;
}

} // namespace astraeus

#endif // ASTRAEUS_WORLD_HPP
