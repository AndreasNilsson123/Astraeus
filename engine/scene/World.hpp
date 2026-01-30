#ifndef ASTRAEUS_WORLD_HPP
#define ASTRAEUS_WORLD_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include "Camera.hpp"

namespace astraeus {

/**
 * Entity transform (SoA component storage).
 */
struct Transform {
    float pos_x, pos_y, pos_z;
    float rot_x, rot_y, rot_z;
    float scale_x, scale_y, scale_z;
    
    // World matrix (computed from local transform + parent hierarchy)
    float world_matrix[16];  // 4x4 matrix, column-major
    bool dirty;              // Needs recomputation

    Transform()
        : pos_x(0), pos_y(0), pos_z(0)
        , rot_x(0), rot_y(0), rot_z(0)
        , scale_x(1), scale_y(1), scale_z(1)
        , dirty(true)
    {
        // Initialize world matrix to identity
        for (int i = 0; i < 16; ++i) world_matrix[i] = 0.0f;
        world_matrix[0] = world_matrix[5] = world_matrix[10] = world_matrix[15] = 1.0f;
    }
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
 * EntityName component - human-readable name for entity.
 */
struct EntityName {
    char name[64];
    
    EntityName() {
        name[0] = '\0';
    }
    
    explicit EntityName(const char* n) {
        if (n) {
            strncpy(name, n, 63);
            name[63] = '\0';
        } else {
            name[0] = '\0';
        }
    }
};

/**
 * Hierarchy component - parent-child relationships.
 */
struct Hierarchy {
    uint32_t parent;                 // Parent entity ID (0 = root)
    std::vector<uint32_t> children;  // Child entity IDs
    
    Hierarchy() : parent(0) {}
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
     * Set entity name.
     */
    void set_entity_name(uint32_t entity_id, const char* name);
    
    /**
     * Get entity name.
     */
    const char* get_entity_name(uint32_t entity_id) const;
    
    /**
     * Find entity by name (returns 0 if not found).
     */
    uint32_t find_entity_by_name(const char* name) const;
    
    /**
     * Set entity parent (use 0 for root).
     */
    void set_entity_parent(uint32_t entity_id, uint32_t parent_id);
    
    /**
     * Get entity parent (returns 0 if root).
     */
    uint32_t get_entity_parent(uint32_t entity_id) const;
    
    /**
     * Get entity children.
     */
    const std::vector<uint32_t>* get_entity_children(uint32_t entity_id) const;
    
    /**
     * Update world transforms for all entities (propagates parent transforms).
     * Call once per frame before rendering.
     */
    void update_world_transforms();
    
    /**
     * Get entity world matrix (4x4, column-major).
     */
    const float* get_entity_world_matrix(uint32_t entity_id) const;
    
    /**
     * Compute bounding box for entity (stub for now).
     */
    struct AABB {
        float min_x, min_y, min_z;
        float max_x, max_y, max_z;
    };
    bool get_entity_bounding_box(uint32_t entity_id, AABB& out_aabb) const;
    
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
    std::unordered_map<uint32_t, EntityName> names_;
    std::unordered_map<uint32_t, Hierarchy> hierarchies_;
    std::vector<uint32_t> active_entities_;
    std::vector<uint32_t> renderable_entities_cache_;
    
    Camera camera_;
    bool is_initialized_;
    
    // Helper methods for transform propagation
    void update_entity_world_transform(uint32_t entity_id);
    void mark_descendants_dirty(uint32_t entity_id);
    void compute_local_matrix(const Transform& t, float* out_matrix);
    void matrix_multiply(const float* a, const float* b, float* out);
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
    names_.clear();
    hierarchies_.clear();
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

    // Remove from parent's children list
    auto hierarchy_it = hierarchies_.find(entity_id);
    if (hierarchy_it != hierarchies_.end() && hierarchy_it->second.parent != 0) {
        auto parent_hierarchy_it = hierarchies_.find(hierarchy_it->second.parent);
        if (parent_hierarchy_it != hierarchies_.end()) {
            auto& children = parent_hierarchy_it->second.children;
            auto child_it = std::find(children.begin(), children.end(), entity_id);
            if (child_it != children.end()) {
                children.erase(child_it);
            }
        }
    }
    
    // Reparent children to root (or could delete them recursively)
    if (hierarchy_it != hierarchies_.end()) {
        for (uint32_t child_id : hierarchy_it->second.children) {
            auto child_hierarchy_it = hierarchies_.find(child_id);
            if (child_hierarchy_it != hierarchies_.end()) {
                child_hierarchy_it->second.parent = 0;
            }
        }
    }

    // Remove all components
    transforms_.erase(entity_id);
    renderables_.erase(entity_id);
    colors_.erase(entity_id);
    trails_.erase(entity_id);
    labels_.erase(entity_id);
    names_.erase(entity_id);
    hierarchies_.erase(entity_id);
    
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
        t.dirty = true;  // Mark as needing recomputation
        
        // Mark all descendants as dirty recursively
        mark_descendants_dirty(entity_id);
    }
}

inline void World::mark_descendants_dirty(uint32_t entity_id) {
    auto hier_it = hierarchies_.find(entity_id);
    if (hier_it != hierarchies_.end()) {
        for (uint32_t child_id : hier_it->second.children) {
            auto child_trans = transforms_.find(child_id);
            if (child_trans != transforms_.end()) {
                child_trans->second.dirty = true;
            }
            // Recursively mark grandchildren
            mark_descendants_dirty(child_id);
        }
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

inline void World::set_entity_name(uint32_t entity_id, const char* name) {
    if (entity_id == 0) {
        return;
    }
    
    names_[entity_id] = EntityName(name);
}

inline const char* World::get_entity_name(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = names_.find(entity_id);
    if (it != names_.end()) {
        return it->second.name;
    }
    return nullptr;
}

inline uint32_t World::find_entity_by_name(const char* name) const {
    if (!name || name[0] == '\0') {
        return 0;
    }
    
    for (const auto& pair : names_) {
        if (strcmp(pair.second.name, name) == 0) {
            return pair.first;
        }
    }
    return 0;
}

inline void World::set_entity_parent(uint32_t entity_id, uint32_t parent_id) {
    if (entity_id == 0) {
        return;
    }
    
    // Don't allow self-parenting or circular references (simple check)
    if (entity_id == parent_id) {
        return;
    }
    
    // Remove from old parent's children list
    auto it = hierarchies_.find(entity_id);
    if (it != hierarchies_.end() && it->second.parent != 0) {
        auto old_parent_it = hierarchies_.find(it->second.parent);
        if (old_parent_it != hierarchies_.end()) {
            auto& children = old_parent_it->second.children;
            auto child_it = std::find(children.begin(), children.end(), entity_id);
            if (child_it != children.end()) {
                children.erase(child_it);
            }
        }
    }
    
    // Create hierarchy component if needed
    if (it == hierarchies_.end()) {
        hierarchies_[entity_id] = Hierarchy();
        it = hierarchies_.find(entity_id);
    }
    
    // Set new parent
    it->second.parent = parent_id;
    
    // Add to new parent's children list
    if (parent_id != 0) {
        auto parent_it = hierarchies_.find(parent_id);
        if (parent_it == hierarchies_.end()) {
            hierarchies_[parent_id] = Hierarchy();
            parent_it = hierarchies_.find(parent_id);
        }
        parent_it->second.children.push_back(entity_id);
    }
    
    // Mark transform as dirty
    auto trans_it = transforms_.find(entity_id);
    if (trans_it != transforms_.end()) {
        trans_it->second.dirty = true;
    }
}

inline uint32_t World::get_entity_parent(uint32_t entity_id) const {
    if (entity_id == 0) {
        return 0;
    }
    
    auto it = hierarchies_.find(entity_id);
    if (it != hierarchies_.end()) {
        return it->second.parent;
    }
    return 0;
}

inline const std::vector<uint32_t>* World::get_entity_children(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = hierarchies_.find(entity_id);
    if (it != hierarchies_.end()) {
        return &it->second.children;
    }
    return nullptr;
}

inline void World::matrix_multiply(const float* a, const float* b, float* out) {
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
    std::memcpy(out, temp, 16 * sizeof(float));
}

inline void World::compute_local_matrix(const Transform& t, float* out_matrix) {
    // Build TRS matrix: Translation * Rotation * Scale
    // For simplicity, we'll do a basic implementation without full quaternion support
    // This assumes rotation is in Euler angles (degrees)
    
    // Convert rotation to radians
    const float PI = 3.14159265359f;
    float rx = t.rot_x * PI / 180.0f;
    float ry = t.rot_y * PI / 180.0f;
    float rz = t.rot_z * PI / 180.0f;
    
    // Compute rotation matrix (ZYX order)
    float cx = std::cos(rx), sx = std::sin(rx);
    float cy = std::cos(ry), sy = std::sin(ry);
    float cz = std::cos(rz), sz = std::sin(rz);
    
    // Combined rotation matrix
    float r00 = cy * cz;
    float r01 = cy * sz;
    float r02 = -sy;
    
    float r10 = sx * sy * cz - cx * sz;
    float r11 = sx * sy * sz + cx * cz;
    float r12 = sx * cy;
    
    float r20 = cx * sy * cz + sx * sz;
    float r21 = cx * sy * sz - sx * cz;
    float r22 = cx * cy;
    
    // Build final matrix with scale and translation (column-major)
    out_matrix[0]  = r00 * t.scale_x;
    out_matrix[1]  = r10 * t.scale_x;
    out_matrix[2]  = r20 * t.scale_x;
    out_matrix[3]  = 0.0f;
    
    out_matrix[4]  = r01 * t.scale_y;
    out_matrix[5]  = r11 * t.scale_y;
    out_matrix[6]  = r21 * t.scale_y;
    out_matrix[7]  = 0.0f;
    
    out_matrix[8]  = r02 * t.scale_z;
    out_matrix[9]  = r12 * t.scale_z;
    out_matrix[10] = r22 * t.scale_z;
    out_matrix[11] = 0.0f;
    
    out_matrix[12] = t.pos_x;
    out_matrix[13] = t.pos_y;
    out_matrix[14] = t.pos_z;
    out_matrix[15] = 1.0f;
}

inline void World::update_entity_world_transform(uint32_t entity_id) {
    auto trans_it = transforms_.find(entity_id);
    if (trans_it == transforms_.end()) {
        return;
    }
    
    Transform& t = trans_it->second;
    
    // Skip if not dirty
    if (!t.dirty) {
        return;
    }
    
    // Compute local matrix
    float local_matrix[16];
    compute_local_matrix(t, local_matrix);
    
    // Check for parent
    auto hier_it = hierarchies_.find(entity_id);
    if (hier_it != hierarchies_.end() && hier_it->second.parent != 0) {
        // Has parent - multiply by parent's world matrix
        auto parent_trans_it = transforms_.find(hier_it->second.parent);
        if (parent_trans_it != transforms_.end()) {
            // Ensure parent is up to date first (recursive)
            update_entity_world_transform(hier_it->second.parent);
            
            // world = parent_world * local
            matrix_multiply(parent_trans_it->second.world_matrix, local_matrix, t.world_matrix);
        } else {
            // Parent not found, use local as world
            std::memcpy(t.world_matrix, local_matrix, 16 * sizeof(float));
        }
    } else {
        // No parent - local is world
        std::memcpy(t.world_matrix, local_matrix, 16 * sizeof(float));
    }
    
    t.dirty = false;
}

inline void World::update_world_transforms() {
    // Update all entities that are dirty
    // We process in hierarchy order implicitly via the recursive update
    for (uint32_t entity_id : active_entities_) {
        update_entity_world_transform(entity_id);
    }
}

inline const float* World::get_entity_world_matrix(uint32_t entity_id) const {
    if (entity_id == 0) {
        return nullptr;
    }
    
    auto it = transforms_.find(entity_id);
    if (it != transforms_.end()) {
        return it->second.world_matrix;
    }
    return nullptr;
}

inline bool World::get_entity_bounding_box(uint32_t entity_id, AABB& out_aabb) const {
    if (entity_id == 0) {
        return false;
    }
    
    // Stub implementation: return a unit cube at the entity's position
    auto trans_it = transforms_.find(entity_id);
    if (trans_it != transforms_.end()) {
        const Transform& t = trans_it->second;
        
        // Simple bounding box based on position and scale
        float half_x = t.scale_x * 0.5f;
        float half_y = t.scale_y * 0.5f;
        float half_z = t.scale_z * 0.5f;
        
        out_aabb.min_x = t.pos_x - half_x;
        out_aabb.min_y = t.pos_y - half_y;
        out_aabb.min_z = t.pos_z - half_z;
        
        out_aabb.max_x = t.pos_x + half_x;
        out_aabb.max_y = t.pos_y + half_y;
        out_aabb.max_z = t.pos_z + half_z;
        
        return true;
    }
    
    return false;
}

} // namespace astraeus

#endif // ASTRAEUS_WORLD_HPP
